/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/hw/sai/api/SwitchApi.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"

#include <folly/logging/xlog.h>

#include <gtest/gtest.h>

using namespace facebook::fboss;

class SwitchApiTest : public ::testing::Test {
 public:
  void SetUp() override {
    fs = FakeSai::getInstance();
    sai_api_initialize(0, nullptr);
    switchApi = std::make_unique<SwitchApi>();
    switchId = SwitchSaiId{fs->swm.create(FakeSwitch())};
  }
  std::shared_ptr<FakeSai> fs;
  std::unique_ptr<SwitchApi> switchApi;
  SwitchSaiId switchId;
};

TEST_F(SwitchApiTest, setGetInit) {
  SaiSwitchTraits::Attributes::InitSwitch init{true};
  switchApi->setAttribute2(switchId, init);
  SaiSwitchTraits::Attributes::InitSwitch blank{false};
  EXPECT_TRUE(switchApi->getAttribute2(switchId, blank));
}

TEST_F(SwitchApiTest, getNumPorts) {
  SaiSwitchTraits::Attributes::PortNumber pn;
  // expect the one global cpu port
  EXPECT_EQ(switchApi->getAttribute2(switchId, pn), 1);
  fs->pm.create(FakePort{{0}, 100000});
  fs->pm.create(FakePort{{1}, 25000});
  fs->pm.create(FakePort{{2}, 25000});
  fs->pm.create(FakePort{{3}, 25000});
  // expect 4 created ports plus global cpu port
  EXPECT_EQ(switchApi->getAttribute2(switchId, pn), 5);
}

TEST_F(SwitchApiTest, setNumPorts) {
  SaiSwitchTraits::Attributes::PortNumber pn{100};
  EXPECT_THROW(
      saiCheckError(switchApi->setAttribute2(switchId, pn)), SaiApiError);
}

TEST_F(SwitchApiTest, testGetPortIds) {
  fs->pm.create(FakePort{{0}, 100000});
  fs->pm.create(FakePort{{1}, 25000});
  fs->pm.create(FakePort{{2}, 25000});
  fs->pm.create(FakePort{{3}, 25000});
  SaiSwitchTraits::Attributes::PortNumber pn;
  auto numPorts = switchApi->getAttribute2(
      switchId, SaiSwitchTraits::Attributes::PortNumber());
  std::vector<sai_object_id_t> v;
  v.resize(numPorts);
  SaiSwitchTraits::Attributes::PortList pl(v);
  auto portIds = switchApi->getAttribute2(switchId, pl);
  EXPECT_EQ(portIds.size(), numPorts);
}

TEST_F(SwitchApiTest, testSetMac) {
  folly::MacAddress newSrcMac("DE:AD:BE:EF:42:42");
  SaiSwitchTraits::Attributes::SrcMac ma(newSrcMac);
  switchApi->setAttribute2(switchId, ma);
  SaiSwitchTraits::Attributes::SrcMac blank;
  EXPECT_EQ(switchApi->getAttribute2(switchId, blank), newSrcMac);
}

TEST_F(SwitchApiTest, getDefaultVlanId) {
  EXPECT_EQ(
      switchApi->getAttribute2(
          switchId, SaiSwitchTraits::Attributes::DefaultVlanId()),
      0);
}

TEST_F(SwitchApiTest, setDefaultVlanId) {
  EXPECT_EQ(
      switchApi->setAttribute2(
          switchId, SaiSwitchTraits::Attributes::DefaultVlanId(42)),
      SAI_STATUS_INVALID_PARAMETER);
}

TEST_F(SwitchApiTest, getCpuPort) {
  auto cpuPort = switchApi->getAttribute2(
      switchId, SaiSwitchTraits::Attributes::CpuPort{});
  EXPECT_EQ(cpuPort, 0);
}
