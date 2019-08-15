/* Copyright 2018 Alibaba Group. All Rights Reserved.

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

#include "ps-plus/client/partitioner/broadcast.h"
#include "ps-plus/client/partitioner/reduce.h"
#include "xdl/core/utils/string_utils.h"
#include "xdl/core/lib/status.h"
#include "xdl/core/framework/op_kernel.h"
#include "xdl/core/framework/op_define.h"
#include "xdl/core/framework/op_registry.h"
#include "xdl/core/ops/ps_ops/define_op.h"
#include "xdl/core/ops/ps_ops/convert_utils.h"
#include "xdl/core/ops/ps_ops/client.h"
#include "xdl/core/ops/ps_ops/var_type.h"
#include <tbb/concurrent_queue.h>
#include "xdl/core/lib/tbb_concurrent_queue.h"

namespace xdl {

class DequeueOp : public xdl::OpKernelAsync {
 public:
  Status Init(OpKernelConstruction* ctx) override {
    return Status::Ok();
  }

  void Compute(OpKernelContext* ctx, Callback done) override {
    std::vector<Tensor> tensors;
    bool success = false;
    while (!TBBConcurrentQueue::Global()->Finished() || TBBConcurrentQueue::Global()->Raw()->size() != 0) {
      if (TBBConcurrentQueue::Global()->Raw()->try_pop(tensors)) {
        success = true;
        break;
      }
    }
    if (success) {
      XDL_CHECK_STATUS_ASYNC(ctx->SetOutputList("tensors", tensors), done);
      done(Status::Ok());      
    } else {
      done(Status::OutOfRange("Dequeue finished"));      
    }
  }

 private:
  std::vector<std::string> names_;
};

XDL_DEFINE_OP(DequeueOp)
  .OutputListV2("tensors", "types")
  .Attr("types", AttrValue::kDataTypeList);

XDL_REGISTER_KERNEL(DequeueOp, DequeueOp).Device("CPU");

} // namespace xdl


