/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include "fboss/agent/hw/sai/api/SaiApiTable.h"
#include "fboss/agent/hw/sai/api/VirtualRouterApi.h"
#include "fboss/agent/types.h"

#include "folly/MacAddress.h"

#include <memory>
#include <unordered_map>

namespace facebook {
namespace fboss {

class SaiManagerTable;
class SaiPlatform;

class SaiSwitchInstance {
 public:
  explicit SaiSwitchInstance(
      const SaiSwitchTraits::CreateAttributes& attributes);
  ~SaiSwitchInstance();
  SaiSwitchInstance(const SaiSwitchInstance& other) = delete;
  SaiSwitchInstance(SaiSwitchInstance&& other) = delete;
  SaiSwitchInstance& operator=(const SaiSwitchInstance& other) = delete;
  SaiSwitchInstance& operator=(SaiSwitchInstance&& other) = delete;
  bool operator==(const SaiSwitchInstance& other) const;
  bool operator!=(const SaiSwitchInstance& other) const;

  const SaiSwitchTraits::CreateAttributes attributes() const {
    return attributes_;
  }
  SwitchSaiId id() const {
    return id_;
  }

 private:
  SaiSwitchTraits::CreateAttributes attributes_;
  SwitchSaiId id_;
};

class SaiSwitchManager {
 public:
  SaiSwitchManager(SaiManagerTable* managerTable, SaiPlatform* platform);
  const SaiSwitchInstance* getSwitch() const;
  SaiSwitchInstance* getSwitch();
  SwitchSaiId getSwitchSaiId() const;

 private:
  SaiSwitchInstance* getSwitchImpl() const;
  SaiManagerTable* managerTable_;
  const SaiPlatform* platform_;
  std::unique_ptr<SaiSwitchInstance> switch_;
};

} // namespace fboss
} // namespace facebook
