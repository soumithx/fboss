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

#include "fboss/agent/FbossError.h"
#include "fboss/agent/hw/sai/api/SaiApiTable.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"
#include "fboss/agent/hw/sai/switch/SaiManagerTable.h"
#include "fboss/agent/state/Route.h"
#include "fboss/agent/types.h"

#include <array>
#include <string>

#include <gtest/gtest.h>

extern "C" {
#include <sai.h>
}

namespace facebook {
namespace fboss {

class ArpEntry;
class Interface;
class Port;
class ResolvedNextHop;
class Vlan;

class ManagerTestBase : public ::testing::Test {
 public:
  // Using a plain enum (rather than enum class) because the setup treats
  // them as a bitmask.
  enum SetupStage : uint32_t {
    BLANK = 0,
    PORT = 1,
    VLAN = 2,
    INTERFACE = 4,
    NEIGHBOR = 8,
  };

  /*
   * TestPort, TestRemoteHost, and TestInterface are helper structs for
   * creating a really basic test setup consistently across the SAI manager
   * tests. They are not strictly necessary at all, but housing the data this
   * way made writing quick setup methods easier than config or creating
   * SwitchState objects directly. If these start getting complex, we
   * SHOULD get rid of them and use one of those clumsier, but more direct
   * interfaces directly.
   */
  struct TestPort {
    int id{0};
    bool enabled{true};
    cfg::PortSpeed portSpeed{cfg::PortSpeed::TWENTYFIVEG};
  };
  struct TestRemoteHost {
    int id{0};
    TestPort port;
    folly::IPAddress ip;
    folly::MacAddress mac;
  };
  struct TestInterface {
    int id{0};
    folly::MacAddress routerMac;
    folly::IPAddress routerIp;
    folly::CIDRNetwork subnet;
    std::vector<TestRemoteHost> remoteHosts;
    TestInterface() {}
    TestInterface(int id, size_t numHosts) : id(id) {
      if (id > 9) {
        XLOG(FATAL) << "TestInterface doesn't support id >9";
      }
      if (numHosts > 9) {
        XLOG(FATAL) << "TestInterface doesn't support >9 attached hosts";
      }
      routerMac = folly::MacAddress{folly::sformat("42:42:42:42:42:0{}", id)};
      auto subnetBase = folly::sformat("10.10.1{}", id);
      routerIp = folly::IPAddress{folly::sformat("{}.0", subnetBase)};
      subnet = folly::CIDRNetwork{routerIp, 24};
      remoteHosts.resize(numHosts);
      size_t count = 0;
      for (auto& remoteHost : remoteHosts) {
        remoteHost.ip =
            folly::IPAddress{folly::sformat("{}.{}", subnetBase, count)};
        remoteHost.mac =
            folly::MacAddress{folly::sformat("10:10:10:10:10:0{}", count)};
        remoteHost.port.id = id * 10 + count;
        ++count;
      }
    }
    explicit TestInterface(int id) : TestInterface(id, 1) {}
  };
  struct TestRoute {
    folly::CIDRNetwork destination;
    std::vector<TestInterface> nextHopInterfaces;
  };

  void SetUp() override;

  std::shared_ptr<ArpEntry> makePendingArpEntry(
      int id,
      const TestRemoteHost& testRemoteHost) const;

  std::shared_ptr<ArpEntry> makeArpEntry(
      int id,
      const TestRemoteHost& testRemoteHost) const;

  std::shared_ptr<Interface> makeInterface(
      const TestInterface& testInterface) const;

  ResolvedNextHop makeNextHop(const TestInterface& testInterface) const;

  std::shared_ptr<Port> makePort(const TestPort& testPort) const;

  std::shared_ptr<Route<folly::IPAddressV4>> makeRoute(
      const TestRoute& route) const;

  std::shared_ptr<Vlan> makeVlan(const TestInterface& testInterface) const;

  std::shared_ptr<FakeSai> fs;
  // TODO - Add FakeSaiPlatform
  std::shared_ptr<SaiPlatform> saiPlatform;
  std::shared_ptr<SaiApiTable> saiApiTable;
  std::unique_ptr<SaiManagerTable> saiManagerTable;

  std::array<TestInterface, 10> testInterfaces;
  uint32_t setupStage{SetupStage::BLANK};
};

} // namespace fboss
} // namespace facebook
