/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/platforms/sai/SaiPlatformInit.h"
#include "fboss/agent/platforms/common/PlatformProductInfo.h"

#include <memory>

#include "fboss/agent/AgentConfig.h"
#include "fboss/agent/Platform.h"
#include "fboss/agent/platforms/sai/SaiBcmPlatform.h"

namespace facebook {
namespace fboss {

std::unique_ptr<SaiPlatform> chooseSaiPlatform(
    std::unique_ptr<PlatformProductInfo> productInfo) {
  if (productInfo->getMode() == PlatformMode::WEDGE100) {
    return std::make_unique<SaiBcmPlatform>(std::move(productInfo));
  }
  return chooseFBSaiPlatform(std::move(productInfo));
}

std::unique_ptr<Platform> initSaiPlatform(std::unique_ptr<AgentConfig> config) {
  auto productInfo =
      std::make_unique<PlatformProductInfo>(FLAGS_fruid_filepath);
  productInfo->initialize();

  auto platform = chooseSaiPlatform(std::move(productInfo));
  platform->init(std::move(config));
  return std::move(platform);
}
} // namespace fboss
} // namespace facebook
