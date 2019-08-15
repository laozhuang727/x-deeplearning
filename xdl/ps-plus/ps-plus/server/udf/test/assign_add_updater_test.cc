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

#include "gtest/gtest.h"
#include "ps-plus/common/data.h"
#include "ps-plus/server/udf.h"
#include "ps-plus/server/slice.h"
#include "ps-plus/common/initializer/constant_initializer.h"
#include "ps-plus/common/hashmap.h"

using ps::server::Udf;
using ps::server::UdfContext;
using ps::server::UdfRegistry;
using ps::server::Variable;
using ps::server::Slices;
using ps::initializer::ConstantInitializer;
using ps::Initializer;
using ps::DataType;
using ps::TensorShape;
using ps::Tensor;
using ps::Data;
using ps::WrapperData;
using ps::HashMap;
using std::vector;

TEST(AssignAddUpdater, AssignAddUpdater) {
  UdfRegistry* udf_registry = UdfRegistry::Get("AssignAddUpdater");
  Udf* udf = udf_registry->Build(std::vector<size_t>({0, 1}), std::vector<size_t>({}));
  UdfContext* ctx = new UdfContext;
  Variable* var = new Variable(new Tensor(DataType::kFloat, TensorShape({4, 8}), new ConstantInitializer(5), true, 1), nullptr, "");
  ctx->SetVariable(var);
  vector<Slices> slices(1, Slices{.slice_size = 8, .slice_id = std::vector<size_t>({0, 2}), .dim_part = 1, .variable = var, .writable = true});
  vector<Tensor> grad(1, Tensor(DataType::kFloat, TensorShape({2, 8}), new ConstantInitializer(2)));
  ctx->SetData(0, new WrapperData<vector<Slices> >(slices), true);
  ctx->SetData(1, new WrapperData<vector<Tensor> >(grad), true);  

  EXPECT_TRUE(udf->Run(ctx).IsOk());
  for (size_t i = 0; i < 8; i++) {
    EXPECT_EQ(7, *(var->GetData()->Raw<float>(0) + i));
    EXPECT_EQ(7, *(var->GetData()->Raw<float>(2) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(1) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(3) + i));
  }
  vector<Tensor> grad2(1, Tensor(DataType::kFloat, TensorShape({2, 8}), new ConstantInitializer(4)));    
  ctx->SetData(1, new WrapperData<vector<Tensor> >(grad2), true);  
  EXPECT_TRUE(udf->Run(ctx).IsOk());
  for (size_t i = 0; i < 8; i++) {
    EXPECT_EQ(11, *(var->GetData()->Raw<float>(0) + i));
    EXPECT_EQ(11, *(var->GetData()->Raw<float>(2) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(1) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(3) + i));        
  }

  vector<Slices> slices2(1, Slices{.slice_size = 8, .slice_id = std::vector<size_t>({(size_t)ps::HashMap::NOT_ADD_ID, 2}), .dim_part = -1, .variable = var, .writable = true});
  ctx->SetData(0, new WrapperData<vector<Slices> >(slices2), true);
  EXPECT_TRUE(udf->Run(ctx).IsOk());
  for (size_t i = 0; i < 8; i++) {
    EXPECT_EQ(11, *(var->GetData()->Raw<float>(0) + i));
    EXPECT_EQ(15, *(var->GetData()->Raw<float>(2) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(1) + i));
    EXPECT_EQ(5, *(var->GetData()->Raw<float>(3) + i));
  }

  vector<Slices> slices3(1, Slices{.slice_size = 8, .slice_id = std::vector<size_t>({(size_t)ps::HashMap::NOT_ADD_ID, 1}), .dim_part = -1, .variable = var, .writable = false});
  ctx->SetData(0, new WrapperData<vector<Slices> >(slices3), true);
  ps::Status status = udf->Run(ctx);
  EXPECT_FALSE(status.IsOk());
  EXPECT_EQ(status.Msg(), "slice is not writable");

  vector<Tensor> grad4(1, Tensor(DataType::kDouble, TensorShape({2, 8}), new ConstantInitializer(6)));    
  ctx->SetData(0, new WrapperData<vector<Slices> >(slices2), true);
  ctx->SetData(1, new WrapperData<vector<Tensor> >(grad4), true);
  status = udf->Run(ctx);
  EXPECT_FALSE(status.IsOk());
  EXPECT_EQ(status.Msg(), "grad should has same datatype with variable");      
  delete var;
  delete ctx;
  delete udf;
}

