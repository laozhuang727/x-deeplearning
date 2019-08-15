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

namespace xdl {

class ConcatOp : public xdl::OpKernel {
 public:
  Status Init(OpKernelConstruction* ctx) override {
    XDL_CHECK_STATUS(ctx->GetAttr("axis", &axis_));
    XDL_CHECK_STATUS(ctx->GetAttr("dtype", &dtype_));
    return Status::Ok();
  }

  Status Compute(OpKernelContext* ctx) override {
    std::vector<Tensor> tensors;
    XDL_CHECK_STATUS(ctx->GetInputList("tensors", &tensors));
    if (tensors.size() == 0) {
      return Status::ArgumentError("Concat should have at least 1 tensor");
    }
    std::vector<size_t> dims = tensors[0].Shape().Dims();
    int64_t real_axis;
    if (axis_ < 0) {
      if (-axis_ > dims.size()) {
        return Status::ArgumentError("Concat axis_ should be [-value_dims, value_dims)");
      }
      real_axis = dims.size() + axis_;
    } else {
      if (axis_ >= dims.size()) {
        return Status::ArgumentError("Concat axis_ should be [-value_dims, value_dims)");
      }
      real_axis = axis_;
    }
    dims[real_axis] = 0;
    std::vector<int64_t> slice;
    for (auto&& item : tensors) {
      if (item.Shape().Size() != dims.size()) {
        return Status::ArgumentError("Concat dims size mismatch");
      }
      for (size_t i = 0; i < dims.size(); i++) {
        if (i == real_axis) {
          continue;
        }
        if (item.Shape()[i] != dims[i]) {
          return Status::ArgumentError("Concat dims mismatch");
        }
      }
      dims[real_axis] += item.Shape()[real_axis];
      slice.push_back(item.Shape()[real_axis]);
    }
    int64_t repeat = 1, one_slice = SizeOfType(dtype_);
    for (size_t i = 0; i < real_axis; i++) {
      repeat *= dims[i];
    }
    for (size_t i = real_axis + 1; i < dims.size(); i++) {
      one_slice *= dims[i];
    }
    for (auto&& item : slice) {
      item *= one_slice;
    }
    Tensor output;
    XDL_CHECK_STATUS(ctx->AllocateOutput(0, TensorShape(dims), &output));
    std::vector<char*> org_ptr;
    for (auto&& item : tensors) {
      org_ptr.push_back(item.Raw<char>());
    }
    char* ptr = output.Raw<char>();
    for (int64_t i = 0; i < repeat; i++) {
      for (size_t j = 0; j < tensors.size(); j++) {
        memcpy(ptr, org_ptr[j], slice[j]);
        org_ptr[j] += slice[j];
        ptr += slice[j];
      }
    }
    return Status::Ok();
  }
 private:
  int64_t axis_;
  DataType dtype_;
};

XDL_DEFINE_OP(Concat)
  .InputList("tensors", "dtype", "size")
  .Attr("axis", AttrValue::kInt)
  .Attr("dtype", AttrValue::kDataType)
  .Attr("size", AttrValue::kInt)
  .Output("result", "dtype");

XDL_REGISTER_KERNEL(Concat, ConcatOp)
  .Device("CPU");

} // namespace xdl

