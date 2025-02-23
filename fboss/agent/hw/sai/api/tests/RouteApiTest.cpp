/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/hw/sai/api/RouteApi.h"
#include "fboss/agent/hw/sai/api/SaiObjectApi.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"

#include <folly/IPAddress.h>
#include <folly/logging/xlog.h>

#include <gtest/gtest.h>

#include <vector>

using namespace facebook::fboss;

static constexpr folly::StringPiece str4 = "42.42.12.34";
static constexpr folly::StringPiece str6 =
    "4242:4242:4242:4242:1234:1234:1234:1234";
static constexpr folly::StringPiece strMac = "42:42:42:12:34:56";

class RouteApiTest : public ::testing::Test {
 public:
  void SetUp() override {
    fs = FakeSai::getInstance();
    sai_api_initialize(0, nullptr);
    routeApi = std::make_unique<RouteApi>();
  }
  std::shared_ptr<FakeSai> fs;
  std::unique_ptr<RouteApi> routeApi;
  folly::IPAddress ip4{str4};
  folly::IPAddress ip6{str6};
  folly::MacAddress dstMac{strMac};
};

TEST_F(RouteApiTest, create2V4Route) {
  folly::CIDRNetwork prefix(ip4, 24);
  SaiRouteTraits::RouteEntry r(0, 0, prefix);
  SaiRouteTraits::Attributes::PacketAction packetActionAttribute{
      SAI_PACKET_ACTION_FORWARD};
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute(5);
  routeApi->create2<SaiRouteTraits>(
      r, {packetActionAttribute, nextHopIdAttribute});
  EXPECT_EQ(
      routeApi->getAttribute2(r, SaiRouteTraits::Attributes::NextHopId()), 5);
}

TEST_F(RouteApiTest, create2V6Route) {
  folly::CIDRNetwork prefix(ip6, 64);
  SaiRouteTraits::RouteEntry r(0, 0, prefix);
  SaiRouteTraits::Attributes::PacketAction packetActionAttribute{
      SAI_PACKET_ACTION_FORWARD};
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute(5);
  routeApi->create2<SaiRouteTraits>(
      r, {packetActionAttribute, nextHopIdAttribute});
  EXPECT_EQ(
      routeApi->getAttribute2(r, SaiRouteTraits::Attributes::NextHopId()), 5);
}

TEST_F(RouteApiTest, setRouteNextHop) {
  folly::CIDRNetwork prefix(ip4, 24);
  SaiRouteTraits::RouteEntry r(0, 0, prefix);
  SaiRouteTraits::Attributes::PacketAction packetActionAttribute{
      SAI_PACKET_ACTION_FORWARD};
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute(5);
  routeApi->create2<SaiRouteTraits>(
      r, {packetActionAttribute, nextHopIdAttribute});
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute2(0);
  routeApi->setAttribute2(r, nextHopIdAttribute2);
  EXPECT_EQ(
      routeApi->getAttribute2(r, SaiRouteTraits::Attributes::NextHopId()), 0);
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute3(42);
  routeApi->setAttribute2(r, nextHopIdAttribute3);
  EXPECT_EQ(
      routeApi->getAttribute2(r, SaiRouteTraits::Attributes::NextHopId()), 42);
}

TEST_F(RouteApiTest, routeCount) {
  uint32_t count = getObjectCount<SaiRouteTraits>(0);
  EXPECT_EQ(count, 0);
  folly::CIDRNetwork prefix(ip6, 64);
  SaiRouteTraits::RouteEntry r(0, 0, prefix);
  SaiRouteTraits::Attributes::PacketAction packetActionAttribute{
      SAI_PACKET_ACTION_FORWARD};
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute(5);
  routeApi->create2<SaiRouteTraits>(
      r, {packetActionAttribute, nextHopIdAttribute});
  count = getObjectCount<SaiRouteTraits>(0);
  EXPECT_EQ(count, 1);
}

TEST_F(RouteApiTest, routeKeys) {
  folly::CIDRNetwork prefix(ip6, 64);
  SaiRouteTraits::RouteEntry r(0, 0, prefix);
  SaiRouteTraits::Attributes::PacketAction packetActionAttribute{
      SAI_PACKET_ACTION_FORWARD};
  SaiRouteTraits::Attributes::NextHopId nextHopIdAttribute(5);
  routeApi->create2<SaiRouteTraits>(
      r, {packetActionAttribute, nextHopIdAttribute});
  auto routeKeys = getObjectKeys<SaiRouteTraits>(0);
  EXPECT_EQ(routeKeys.size(), 1);
  EXPECT_EQ(routeKeys[0], r);
}
