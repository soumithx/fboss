/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/hw/sai/api/NeighborApi.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"

#include <folly/IPAddress.h>
#include <folly/logging/xlog.h>

#include <gtest/gtest.h>

using namespace facebook::fboss;

static constexpr folly::StringPiece str4 = "42.42.12.34";
static constexpr folly::StringPiece str6 =
    "4242:4242:4242:4242:1234:1234:1234:1234";
static constexpr folly::StringPiece strMac = "42:42:42:12:34:56";

class NeighborApiTest : public ::testing::Test {
 public:
  void SetUp() override {
    fs = FakeSai::getInstance();
    sai_api_initialize(0, nullptr);
    neighborApi = std::make_unique<NeighborApi>();
  }
  std::shared_ptr<FakeSai> fs;
  std::unique_ptr<NeighborApi> neighborApi;
  folly::IPAddress ip4{str4};
  folly::IPAddress ip6{str6};
  folly::MacAddress dstMac{strMac};
};

TEST_F(NeighborApiTest, create2V4Neighbor) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip4);
  FakeNeighborEntry fn = std::make_tuple(0, 0, ip4);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  EXPECT_EQ(fs->nm.get(fn).dstMac, dstMac);
}

TEST_F(NeighborApiTest, create2V6Neighbor) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip6);
  FakeNeighborEntry fn = std::make_tuple(0, 0, ip6);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  EXPECT_EQ(fs->nm.get(fn).dstMac, dstMac);
}

TEST_F(NeighborApiTest, removeV4Neighbor) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip4);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  EXPECT_EQ(fs->nm.map().size(), 1);
  neighborApi->remove2(n);
  EXPECT_EQ(fs->nm.map().size(), 0);
}

TEST_F(NeighborApiTest, removeV6Neighbor) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip6);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  EXPECT_EQ(fs->nm.map().size(), 1);
  neighborApi->remove2(n);
  EXPECT_EQ(fs->nm.map().size(), 0);
}

TEST_F(NeighborApiTest, getV4DstMac) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip4);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  auto gotMac =
      neighborApi->getAttribute2(n, SaiNeighborTraits::Attributes::DstMac());
  EXPECT_EQ(gotMac, dstMac);
}

TEST_F(NeighborApiTest, setV4DstMac) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip4);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  folly::MacAddress dstMac2("22:22:22:22:22:22");
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute2(dstMac2);
  neighborApi->setAttribute2(n, dstMacAttribute2);
  auto gotMac =
      neighborApi->getAttribute2(n, SaiNeighborTraits::Attributes::DstMac());
  EXPECT_EQ(gotMac, dstMac2);
}

TEST_F(NeighborApiTest, setV6DstMac) {
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute(dstMac);
  SaiNeighborTraits::NeighborEntry n(0, 0, ip6);
  neighborApi->create2<SaiNeighborTraits>(n, {dstMacAttribute});
  folly::MacAddress dstMac2("22:22:22:22:22:22");
  SaiNeighborTraits::Attributes::DstMac dstMacAttribute2(dstMac2);
  neighborApi->setAttribute2(n, dstMacAttribute2);
  auto gotMac =
      neighborApi->getAttribute2(n, SaiNeighborTraits::Attributes::DstMac());
  EXPECT_EQ(gotMac, dstMac2);
}
