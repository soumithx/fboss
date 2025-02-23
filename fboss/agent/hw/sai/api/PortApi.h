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

#include "fboss/agent/hw/sai/api/SaiApi.h"
#include "fboss/agent/hw/sai/api/SaiAttribute.h"
#include "fboss/agent/hw/sai/api/SaiAttributeDataTypes.h"
#include "fboss/agent/hw/sai/api/Types.h"

#include <folly/logging/xlog.h>

#include <optional>
#include <tuple>

extern "C" {
#include <sai.h>
}

namespace facebook {
namespace fboss {

class PortApi;

struct SaiPortTraits {
  static constexpr sai_object_type_t ObjectType = SAI_OBJECT_TYPE_PORT;
  using SaiApiT = PortApi;
  struct Attributes {
    using EnumType = sai_port_attr_t;
    using AdminState = SaiAttribute<EnumType, SAI_PORT_ATTR_ADMIN_STATE, bool>;
    using HwLaneList = SaiAttribute<
        EnumType,
        SAI_PORT_ATTR_HW_LANE_LIST,
        std::vector<uint32_t>>;
    using Speed = SaiAttribute<EnumType, SAI_PORT_ATTR_SPEED, sai_uint32_t>;
    using Type = SaiAttribute<EnumType, SAI_PORT_ATTR_TYPE, sai_int32_t>;
    using QosNumberOfQueues = SaiAttribute<
        EnumType,
        SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES,
        sai_uint32_t>;
    using QosQueueList = SaiAttribute<
        EnumType,
        SAI_PORT_ATTR_QOS_QUEUE_LIST,
        std::vector<sai_object_id_t>>;
    using FecMode = SaiAttribute<EnumType, SAI_PORT_ATTR_FEC_MODE, sai_int32_t>;
    using OperStatus =
        SaiAttribute<EnumType, SAI_PORT_ATTR_OPER_STATUS, sai_int32_t>;
    using InternalLoopbackMode = SaiAttribute<
        EnumType,
        SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE,
        sai_int32_t>;
    using MediaType =
        SaiAttribute<EnumType, SAI_PORT_ATTR_MEDIA_TYPE, sai_int32_t>;
    using GlobalFlowControlMode = SaiAttribute<
        EnumType,
        SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE,
        sai_int32_t>;
    using PortVlanId =
        SaiAttribute<EnumType, SAI_PORT_ATTR_PORT_VLAN_ID, sai_uint16_t>;
  };
  using AdapterKey = PortSaiId;
  using AdapterHostKey = Attributes::HwLaneList;

  using CreateAttributes = std::tuple<
      Attributes::HwLaneList,
      Attributes::Speed,
      std::optional<Attributes::AdminState>,
      std::optional<Attributes::FecMode>,
      std::optional<Attributes::InternalLoopbackMode>,
      std::optional<Attributes::MediaType>,
      std::optional<Attributes::GlobalFlowControlMode>,
      std::optional<Attributes::PortVlanId>>;
};

class PortApi : public SaiApi<PortApi> {
 public:
  static constexpr sai_api_t ApiType = SAI_API_PORT;
  PortApi() {
    sai_status_t status =
        sai_api_query(ApiType, reinterpret_cast<void**>(&api_));
    saiApiCheckError(status, ApiType, "Failed to query for port api");
  }

 private:
  sai_status_t _create(
      PortSaiId* id,
      sai_object_id_t switch_id,
      size_t count,
      sai_attribute_t* attr_list) const {
    return api_->create_port(rawSaiId(id), switch_id, count, attr_list);
  }
  sai_status_t _remove(PortSaiId key) {
    return api_->remove_port(key);
  }
  sai_status_t _getAttribute(PortSaiId key, sai_attribute_t* attr) const {
    return api_->get_port_attribute(key, 1, attr);
  }
  sai_status_t _setAttribute(PortSaiId key, const sai_attribute_t* attr) {
    return api_->set_port_attribute(key, attr);
  }

  sai_port_api_t* api_;
  friend class SaiApi<PortApi>;
};

} // namespace fboss
} // namespace facebook
