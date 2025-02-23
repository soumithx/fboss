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

#include "fboss/agent/platforms/sai/SaiHwPlatform.h"
namespace facebook {
namespace fboss {

class SaiBcmPlatform : public SaiHwPlatform {
 public:
  explicit SaiBcmPlatform(std::unique_ptr<PlatformProductInfo> productInfo)
      : SaiHwPlatform(std::move(productInfo)) {}
  std::vector<PortID> masterLogicalPortIds() const override {
    // TODO
    return {};
  }
  std::string getHwConfig() override;
};

} // namespace fboss
} // namespace facebook
