/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "FakeSaiPort.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"

#include <folly/Optional.h>
#include <folly/logging/xlog.h>

using facebook::fboss::FakePort;
using facebook::fboss::FakeSai;

sai_status_t create_port_fn(
    sai_object_id_t* port_id,
    sai_object_id_t /* switch_id */,
    uint32_t attr_count,
    const sai_attribute_t* attr_list) {
  auto fs = FakeSai::getInstance();
  folly::Optional<bool> adminState;
  std::vector<uint32_t> lanes;
  folly::Optional<sai_uint32_t> speed;
  folly::Optional<sai_port_fec_mode_t> fecMode;
  folly::Optional<sai_port_internal_loopback_mode_t> internalLoopbackMode;
  folly::Optional<sai_port_flow_control_mode_t> flowControlMode;
  folly::Optional<sai_port_media_type_t> mediaType;
  folly::Optional<sai_vlan_id_t> vlanId;
  for (int i = 0; i < attr_count; ++i) {
    switch (attr_list[i].id) {
      case SAI_PORT_ATTR_ADMIN_STATE:
        adminState = attr_list[i].value.booldata;
        break;
      case SAI_PORT_ATTR_HW_LANE_LIST: {
        for (int j = 0; j < attr_list[i].value.u32list.count; ++j) {
          lanes.push_back(attr_list[i].value.u32list.list[j]);
        }
      } break;
      case SAI_PORT_ATTR_SPEED:
        speed = attr_list[i].value.u32;
        break;
      case SAI_PORT_ATTR_FEC_MODE:
        fecMode = static_cast<sai_port_fec_mode_t>(attr_list[i].value.u32);
        break;
      case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
        internalLoopbackMode = static_cast<sai_port_internal_loopback_mode_t>(
            attr_list[i].value.u32);
        break;
      case SAI_PORT_ATTR_MEDIA_TYPE:
        mediaType = static_cast<sai_port_media_type_t>(attr_list[i].value.s32);
        break;
      case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
        flowControlMode =
            static_cast<sai_port_flow_control_mode_t>(attr_list[i].value.s32);
        break;
      case SAI_PORT_ATTR_PORT_VLAN_ID:
        vlanId = attr_list[i].value.u16;
        break;
      default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
  }
  if (lanes.empty() || !speed) {
    return SAI_STATUS_INVALID_PARAMETER;
  }
  *port_id = fs->pm.create(lanes, speed.value());
  auto& port = fs->pm.get(*port_id);
  if (adminState) {
    port.adminState = adminState.value();
  }
  if (fecMode.hasValue()) {
    port.fecMode = fecMode.value();
  }
  if (internalLoopbackMode.hasValue()) {
    port.internalLoopbackMode = internalLoopbackMode.value();
  }
  if (vlanId.hasValue()) {
    port.vlanId = vlanId.value();
  }
  if (mediaType.hasValue()) {
    port.mediaType = mediaType.value();
  }
  if (vlanId.hasValue()) {
    port.vlanId = vlanId.value();
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t remove_port_fn(sai_object_id_t port_id) {
  auto fs = FakeSai::getInstance();
  fs->pm.remove(port_id);
  return SAI_STATUS_SUCCESS;
}

sai_status_t set_port_attribute_fn(
    sai_object_id_t port_id,
    const sai_attribute_t* attr) {
  auto fs = FakeSai::getInstance();
  auto& port = fs->pm.get(port_id);
  sai_status_t res;
  if (!attr) {
    return SAI_STATUS_INVALID_PARAMETER;
  }
  switch (attr->id) {
    case SAI_PORT_ATTR_ADMIN_STATE:
      port.adminState = attr->value.booldata;
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_HW_LANE_LIST: {
      auto& lanes = port.lanes;
      lanes.clear();
      for (int j = 0; j < attr->value.u32list.count; ++j) {
        lanes.push_back(attr->value.u32list.list[j]);
      }
    }
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_SPEED:
      port.speed = attr->value.u32;
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_FEC_MODE:
      port.fecMode = static_cast<sai_port_fec_mode_t>(attr->value.s32);
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
      port.internalLoopbackMode =
          static_cast<sai_port_internal_loopback_mode_t>(attr->value.s32);
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_MEDIA_TYPE:
      port.mediaType = static_cast<sai_port_media_type_t>(attr->value.s32);
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
      port.globalFlowControlMode =
          static_cast<sai_port_flow_control_mode_t>(attr->value.s32);
      res = SAI_STATUS_SUCCESS;
      break;
    case SAI_PORT_ATTR_PORT_VLAN_ID:
      port.vlanId = static_cast<sai_vlan_id_t>(attr->value.u16);
      res = SAI_STATUS_SUCCESS;
      break;
    default:
      res = SAI_STATUS_INVALID_PARAMETER;
      break;
  }
  return res;
}

sai_status_t get_port_attribute_fn(
    sai_object_id_t port_id,
    uint32_t attr_count,
    sai_attribute_t* attr) {
  auto fs = FakeSai::getInstance();
  const auto& port = fs->pm.get(port_id);
  for (int i = 0; i < attr_count; ++i) {
    switch (attr[i].id) {
      case SAI_PORT_ATTR_ADMIN_STATE:
        attr[i].value.booldata = port.adminState;
        break;
      case SAI_PORT_ATTR_HW_LANE_LIST:
        if (port.lanes.size() > attr[i].value.u32list.count) {
          attr[i].value.u32list.count = port.lanes.size();
          return SAI_STATUS_BUFFER_OVERFLOW;
        }
        for (int j = 0; j < port.lanes.size(); ++j) {
          attr[i].value.u32list.list[j] = port.lanes[j];
        }
        attr[i].value.u32list.count = port.lanes.size();
        break;
      case SAI_PORT_ATTR_SPEED:
        attr[i].value.u32 = port.speed;
        break;
      case SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES:
        attr[i].value.u32 = 0;
        break;
      case SAI_PORT_ATTR_QOS_QUEUE_LIST:
        attr[i].value.objlist.count = 0;
        break;
      case SAI_PORT_ATTR_FEC_MODE:
        attr[i].value.s32 = static_cast<int32_t>(port.fecMode);
        break;
      case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
        attr[i].value.s32 = static_cast<int32_t>(port.internalLoopbackMode);
        break;
      case SAI_PORT_ATTR_MEDIA_TYPE:
        attr[i].value.s32 = static_cast<int32_t>(port.mediaType);
        break;
      case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
        attr[i].value.s32 = static_cast<int32_t>(port.globalFlowControlMode);
        break;
      case SAI_PORT_ATTR_PORT_VLAN_ID:
        attr[i].value.u16 = port.vlanId;
        break;
      default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
  }
  return SAI_STATUS_SUCCESS;
}

namespace facebook {
namespace fboss {

static sai_port_api_t _port_api;

void populate_port_api(sai_port_api_t** port_api) {
  _port_api.create_port = &create_port_fn;
  _port_api.remove_port = &remove_port_fn;
  _port_api.set_port_attribute = &set_port_attribute_fn;
  _port_api.get_port_attribute = &get_port_attribute_fn;
  *port_api = &_port_api;
}

} // namespace fboss
} // namespace facebook
