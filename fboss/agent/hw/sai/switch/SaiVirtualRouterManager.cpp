/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/agent/hw/sai/switch/SaiVirtualRouterManager.h"
#include "fboss/agent/hw/sai/store/SaiStore.h"
#include "fboss/agent/hw/sai/switch/SaiSwitchManager.h"

#include "fboss/agent/FbossError.h"
#include "fboss/agent/hw/sai/api/SwitchApi.h"
#include "fboss/agent/hw/sai/switch/SaiManagerTable.h"

#include <folly/logging/xlog.h>

namespace facebook {
namespace fboss {

SaiVirtualRouterManager::SaiVirtualRouterManager(
    SaiManagerTable* managerTable,
    const SaiPlatform* platform)
    : managerTable_(managerTable), platform_(platform) {
  SaiVirtualRouterTraits::AdapterHostKey k;
  SaiVirtualRouterTraits::CreateAttributes attributes;
  auto& store = SaiStore::getInstance()->get<SaiVirtualRouterTraits>();
  auto defaultVirtualRouter = store.setObject(k, attributes);
  auto handle = std::make_unique<SaiVirtualRouterHandle>();
  handle->virtualRouter = defaultVirtualRouter;
  handles_.emplace(std::make_pair(RouterID(0), std::move(handle)));
}

VirtualRouterSaiId SaiVirtualRouterManager::addVirtualRouter(
    const RouterID& /* routerId */) {
  throw FbossError("Adding new virtual routers is not supported");
  folly::assume_unreachable();
}

SaiVirtualRouterHandle* SaiVirtualRouterManager::getVirtualRouterHandle(
    const RouterID& routerId) {
  return getVirtualRouterHandleImpl(routerId);
}

const SaiVirtualRouterHandle* SaiVirtualRouterManager::getVirtualRouterHandle(
    const RouterID& routerId) const {
  return getVirtualRouterHandleImpl(routerId);
}

SaiVirtualRouterHandle* SaiVirtualRouterManager::getVirtualRouterHandleImpl(
    const RouterID& routerId) const {
  auto itr = handles_.find(routerId);
  if (itr == handles_.end()) {
    return nullptr;
  }
  if (!itr->second) {
    XLOG(FATAL) << "invalid null virtual router for routerID: " << routerId;
  }
  return itr->second.get();
}

} // namespace fboss
} // namespace facebook
