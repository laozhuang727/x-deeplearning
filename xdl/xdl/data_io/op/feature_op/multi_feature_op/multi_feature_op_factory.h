/* Copyright (C) 2016-2018 Alibaba Group Holding Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/


#pragma once

#include "xdl/data_io/op/feature_op/multi_feature_op/multi_feature_op.h"

namespace xdl {
namespace io {

class MultiFeatureOpFactory {
 public:
  static MultiFeatureOp *Get(MultiFeaOpType multi_fea_op_type,
                             CombineKeyFunc combine_key_func,
                             CombineValueFunc combine_value_func) {
    MultiFeatureOp *ret = new MultiFeatureOp();
    ret->Init(multi_fea_op_type,
              combine_key_func,
              combine_value_func);
    return ret;
  }

  static void Release(MultiFeatureOp *multi_feature_op) {
    multi_feature_op->Destroy();
    //delete multi_feature_op;
  }
};

}  // namespace io
}  // namespace xdl