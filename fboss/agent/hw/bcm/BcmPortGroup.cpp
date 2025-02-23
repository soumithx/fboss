/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/hw/bcm/BcmPortGroup.h"
#include "fboss/agent/hw/bcm/BcmError.h"
#include "fboss/agent/hw/bcm/BcmPort.h"
#include "fboss/agent/hw/bcm/BcmPortTable.h"
#include "fboss/agent/hw/bcm/BcmSwitch.h"

#include <folly/logging/xlog.h>
#include "fboss/agent/state/Port.h"

namespace {
using facebook::fboss::BcmPortGroup;
using facebook::fboss::FbossError;
using facebook::fboss::LaneSpeeds;
using facebook::fboss::cfg::PortSpeed;
BcmPortGroup::LaneMode neededLaneModeForSpeed(
    PortSpeed speed,
    LaneSpeeds laneSpeeds) {
  if (speed == PortSpeed::DEFAULT) {
    throw FbossError("Speed cannot be DEFAULT if flexports are enabled");
  }

  for (auto& laneSpeed : laneSpeeds) {
    auto dv = std::div(static_cast<int>(speed), static_cast<int>(laneSpeed));
    if (dv.rem) {
      // skip if this requires an unsupported lane speed
      continue;
    }

    auto neededLanes = dv.quot;
    if (neededLanes == 1) {
      return BcmPortGroup::LaneMode::SINGLE;
    } else if (neededLanes == 2) {
      return BcmPortGroup::LaneMode::DUAL;
    } else if (neededLanes > 2 && neededLanes <= 4) {
      return BcmPortGroup::LaneMode::QUAD;
    }
  }

  throw FbossError("Cannot support speed ", speed);
}

} // namespace

namespace facebook {
namespace fboss {

BcmPortGroup::BcmPortGroup(
    BcmSwitch* hw,
    BcmPort* controllingPort,
    std::vector<BcmPort*> allPorts)
    : hw_(hw),
      controllingPort_(controllingPort),
      allPorts_(std::move(allPorts)) {
  if (allPorts_.size() != 4) {
    throw FbossError(
        "Port groups must have exactly four members. Found ", allPorts_.size());
  }

  // We expect all ports to run at the same speed and are passed in in
  // the correct order.
  portSpeed_ = controllingPort_->getSpeed();
  for (int i = 0; i < allPorts_.size(); ++i) {
    auto port = allPorts_[i];
    if (getLane(port) != i) {
      throw FbossError("Ports passed in are not ordered by lane");
    }
    if (port->isEnabled() && portSpeed_ != port->getSpeed()) {
      throw FbossError("All enabled ports must have same speed");
    }
  }

  // get the number of active lanes
  auto activeLanes = retrieveActiveLanes();
  switch (activeLanes) {
    case 1:
      laneMode_ = LaneMode::SINGLE;
      break;
    case 2:
      laneMode_ = LaneMode::DUAL;
      break;
    case 4:
      laneMode_ = LaneMode::QUAD;
      break;
    default:
      throw FbossError(
          "Unexpected number of lanes retrieved for bcm port ",
          controllingPort_->getBcmPortId());
  }
}

BcmPortGroup::~BcmPortGroup() {}

BcmPortGroup::LaneMode BcmPortGroup::calculateDesiredLaneMode(
    const std::vector<Port*>& ports,
    LaneSpeeds laneSpeeds) {
  auto desiredMode = LaneMode::SINGLE;

  for (int lane = 0; lane < ports.size(); ++lane) {
    auto port = ports[lane];
    if (port->isEnabled()) {
      auto neededMode = neededLaneModeForSpeed(port->getSpeed(), laneSpeeds);
      if (neededMode > desiredMode) {
        desiredMode = neededMode;
      }

      if (desiredMode == LaneMode::QUAD) {
        if (lane != 0) {
          throw FbossError("Only lane 0 can be enabled in QUAD mode");
        }
      } else if (desiredMode == LaneMode::DUAL) {
        if (lane != 0 && lane != 2) {
          throw FbossError("Only lanes 0 or 2 can be enabled in DUAL mode");
        }
      }

      XLOG(DBG3) << "Port " << port->getID() << " enabled with speed "
                 << static_cast<int>(port->getSpeed());
    }
  }
  return desiredMode;
}

std::vector<Port*> BcmPortGroup::getSwPorts(
    const std::shared_ptr<SwitchState>& state) const {
  std::vector<Port*> ports;
  for (auto bcmPort : allPorts_) {
    auto swPort = bcmPort->getSwitchStatePort(state).get();
    // Make sure the ports support the configured speed.
    // We check this even if the port is disabled.
    if (!bcmPort->supportsSpeed(swPort->getSpeed())) {
      throw FbossError(
          "Port ",
          swPort->getID(),
          " does not support speed ",
          static_cast<int>(swPort->getSpeed()));
    }
    ports.push_back(swPort);
  }
  return ports;
}

uint8_t BcmPortGroup::getLane(const BcmPort* bcmPort) const {
  return bcmPort->getBcmPortId() - controllingPort_->getBcmPortId();
}

bool BcmPortGroup::validConfiguration(
    const std::shared_ptr<SwitchState>& state) const {
  try {
    calculateDesiredLaneMode(
        getSwPorts(state), controllingPort_->supportedLaneSpeeds());
  } catch (const std::exception& ex) {
    XLOG(DBG1) << "Received exception determining lane mode: " << ex.what();
    return false;
  }
  return true;
}

void BcmPortGroup::reconfigureIfNeeded(
    const std::shared_ptr<SwitchState>& state) {
  // This logic is a bit messy. We could encode some notion of port
  // groups into the swith state somehow so it is easy to generate
  // deltas for these. For now, we need pass around the SwitchState
  // object and get the relevant ports manually.
  auto ports = getSwPorts(state);
  // ports is guaranteed to be the same size as allPorts_
  auto speedChanged = ports[0]->getSpeed() != portSpeed_;
  auto desiredLaneMode =
      calculateDesiredLaneMode(ports, controllingPort_->supportedLaneSpeeds());
  if (speedChanged) {
    controllingPort_->getPlatformPort()->linkSpeedChanged(ports[0]->getSpeed());
  }
  if (desiredLaneMode != laneMode_) {
    reconfigureLaneMode(state, desiredLaneMode);
  }
}

void BcmPortGroup::reconfigureLaneMode(
    const std::shared_ptr<SwitchState>& state,
    LaneMode newLaneMode) {
  // The logic for this follows the steps required for flex-port support
  // outlined in the sdk documentation.
  XLOG(DBG1) << "Reconfiguring port " << controllingPort_->getBcmPortId()
             << " from " << laneMode_ << " active ports to " << newLaneMode
             << " active ports";

  // 1. Disable linkscan, then disable ports.
  for (auto& bcmPort : allPorts_) {
    auto swPort = bcmPort->getSwitchStatePort(state);
    bcmPort->disableLinkscan();
    bcmPort->disable(swPort);
  }

  // 2. Set the opennslPortControlLanes setting
  setActiveLanes(newLaneMode);

  // 3. Enable linkscan, then enable ports.
  for (auto& bcmPort : allPorts_) {
    auto swPort = bcmPort->getSwitchStatePort(state);
    if (swPort->isEnabled()) {
      bcmPort->enableLinkscan();
      bcmPort->enable(swPort);
    }
  }
}

} // namespace fboss
} // namespace facebook
