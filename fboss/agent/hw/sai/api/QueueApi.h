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

class QueueApi;

struct SaiQueueTraits {
  static constexpr sai_object_type_t ObjectType = SAI_OBJECT_TYPE_QUEUE;
  using SaiApiT = QueueApi;
  struct Attributes {
    using EnumType = sai_queue_attr_t;
    using Type = SaiAttribute<EnumType, SAI_QUEUE_ATTR_TYPE, sai_int32_t>;
    using Port = SaiAttribute<EnumType, SAI_QUEUE_ATTR_PORT, SaiObjectIdT>;
    using Index = SaiAttribute<EnumType, SAI_QUEUE_ATTR_INDEX, sai_uint8_t>;
    using ParentSchedulerNode = SaiAttribute<
        EnumType,
        SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE,
        SaiObjectIdT>;
    using WredProfileId =
        SaiAttribute<EnumType, SAI_QUEUE_ATTR_WRED_PROFILE_ID, SaiObjectIdT>;
    using BufferProfileId =
        SaiAttribute<EnumType, SAI_QUEUE_ATTR_BUFFER_PROFILE_ID, SaiObjectIdT>;
    using SchedulerProfileId = SaiAttribute<
        EnumType,
        SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID,
        SaiObjectIdT>;
  };
  using AdapterKey = QueueSaiId;
  using AdapterHostKey =
      std::tuple<Attributes::Type, Attributes::Port, Attributes::Index>;
  using CreateAttributes = std::tuple<
      Attributes::Type,
      Attributes::Port,
      Attributes::Index,
      Attributes::ParentSchedulerNode>;
};

class QueueApi : public SaiApi<QueueApi> {
 public:
  static constexpr sai_api_t ApiType = SAI_API_QUEUE;
  QueueApi() {
    sai_status_t status =
        sai_api_query(ApiType, reinterpret_cast<void**>(&api_));
    saiApiCheckError(status, ApiType, "Failed to query for queue api");
  }

 private:
  sai_status_t _create(
      QueueSaiId* id,
      sai_object_id_t switch_id,
      size_t count,
      sai_attribute_t* attr_list) {
    return api_->create_queue(rawSaiId(id), switch_id, count, attr_list);
  }
  sai_status_t _remove(QueueSaiId id) {
    return api_->remove_queue(id);
  }
  sai_status_t _getAttribute(QueueSaiId id, sai_attribute_t* attr) const {
    return api_->get_queue_attribute(id, 1, attr);
  }
  sai_status_t _setAttribute(QueueSaiId id, const sai_attribute_t* attr) {
    return api_->set_queue_attribute(id, attr);
  }

  sai_queue_api_t* api_;
  friend class SaiApi<QueueApi>;
};

} // namespace fboss
} // namespace facebook
