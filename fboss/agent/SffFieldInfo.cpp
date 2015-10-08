/*
 * Copyright (c) 2004-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "SffFieldInfo.h"
#include "fboss/agent/FbossError.h"

/*
 * Parse transceiver data fields, as outlined in various documents
 * by the Small Form Factor (SFF) committee, including SFP and QSFP+
 * modules.
 */

namespace facebook { namespace fboss {

double SffFieldInfo::getTemp(const uint16_t temp) {
  double data;
  data = temp / 256.0;
  if (data > 128) {
    data = data - 256;
  }
  return data;
}

double SffFieldInfo::getVcc(const uint16_t temp) {
  double data;
  data = temp / 10000.0;
  return data;
}

double SffFieldInfo::getTxBias(const uint16_t temp) {
  double data;
  data = temp * 2.0 / 1000;
  return data;
}

double SffFieldInfo::getPwr(const uint16_t temp) {
  double data;
  data = temp * 0.1 / 1000;
  return data;
}

SffFieldInfo SffFieldInfo::getSffFieldAddress(const SffFieldMap& map,
                                              const SffField field) {
  auto info = map.find(field);
  if (info == map.end()) {
    throw FbossError("Invalid SFF Field ID");
  }
  return info->second;
}


}} //namespace facebook::fboss
