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

#include "xdl/core/lib/status.h"
#include "xdl/core/framework/op_kernel.h"
#include "xdl/core/framework/op_define.h"
#include "xdl/core/framework/op_registry.h"
#include "xdl/core/ops/ps_ops/define_op.h"
#include "xdl/core/ops/ps_ops/convert_utils.h"
#include "xdl/core/ops/ps_ops/client.h"
#include "xdl/core/ops/ps_ops/var_type.h"

namespace xdl {

class PsSparseAssignOp : public xdl::OpKernelAsync {
 public:
  Status Init(OpKernelConstruction* ctx) override {
    XDL_CHECK_STATUS(ctx->GetAttr("var_name", &var_name_));
    XDL_CHECK_STATUS(XdlGetVarType(ctx, &var_type_));
    return Status::Ok();
  }

  void Compute(OpKernelContext* ctx, Callback done) override {
    ps::client::BaseClient* client;
    XDL_CHECK_STATUS_ASYNC(GetClient(&client), done);
    Tensor ids;
    XDL_CHECK_STATUS_ASYNC(ctx->GetInput(0, &ids), done);
    Tensor values;
    XDL_CHECK_STATUS_ASYNC(ctx->GetInput(1, &values), done);    

    ps::Tensor convert_ids;
    XDL_CHECK_STATUS_ASYNC(
        XDL2PS::ConvertTensor(ids, &convert_ids),
        done);
    ps::Tensor convert_values;
    XDL_CHECK_STATUS_ASYNC(
        XDL2PS::ConvertTensor(values, &convert_values),
        done);

    auto cb = [ctx, done](const ps::Status& st) {
      XDL_CHECK_STATUS_ASYNC(PS2XDL::ConvertStatus(st), done);
      done(Status::Ok());
    };

    std::vector<ps::Tensor> value_vec = {convert_values};

    switch(var_type_) {
    case VarType::kIndex:
      client->SparsePush(
          var_name_, 
          convert_ids, 
          "AssignUpdater", 
          client->Args(value_vec), 
          cb);
      break;
    case VarType::kHash128:
    case VarType::kHash64:
      client->HashPush(
          var_name_,
          convert_ids, 
          1.0,
          true,
          "AssignUpdater",
          client->Args(value_vec), 
          cb);
      break;
    }
  }

 private:
  std::string var_name_;
  VarType var_type_;
};

XDL_DEFINE_OP(PsSparseAssignOp)
  .Input("ids", "id_type")
  .Input("values", "value_type")
  .Attr("var_name", AttrValue::kString)
  .Attr("var_type", AttrValue::kString)
  .Attr("id_type", AttrValue::kDataType)
  .Attr("value_type", AttrValue::kDataType);

XDL_REGISTER_KERNEL(PsSparseAssignOp, PsSparseAssignOp).Device("CPU");

} // namespace xdl


