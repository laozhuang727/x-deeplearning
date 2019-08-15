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
#include "ps-plus/common/hashmap.h"
#include "ps-plus/server/udf.h"
#include "ps-plus/server/slice.h"
#include "ps-plus/common/initializer/constant_initializer.h"

using ps::server::Udf;
using ps::server::UdfContext;
using ps::server::UdfRegistry;
using ps::server::Variable;
using ps::server::Slices;
using ps::initializer::ConstantInitializer;
using ps::server::StorageManager;
using ps::Initializer;
using ps::DataType;
using ps::TensorShape;
using ps::Tensor;
using ps::Data;
using ps::WrapperData;
using ps::HashMap;

TEST(HashVariableInitializer, HashVariableInitializer) {
  UdfRegistry* udf_registry = UdfRegistry::Get("HashVariableInitializer");
  Udf* udf = udf_registry->Build(std::vector<size_t>({0, 1, 2, 3}), std::vector<size_t>({}));
  UdfContext* ctx = new UdfContext;
  StorageManager* storage_manager = new StorageManager;
  EXPECT_TRUE(ctx->SetVariableName("123").IsOk());
  EXPECT_TRUE(ctx->SetData(0, new WrapperData<DataType>(DataType::kInt8), true).IsOk());
  EXPECT_TRUE(ctx->SetData(2, new WrapperData<std::string>("save_ratio=1.0&hash64=true"), true).IsOk());  
  EXPECT_TRUE(ctx->SetData(3, new WrapperData<std::unique_ptr<Initializer>>(new ConstantInitializer(1)), true).IsOk());
  EXPECT_TRUE(ctx->SetStorageManager(storage_manager).IsOk());
  ctx->SetData(1, new WrapperData<TensorShape>(std::vector<size_t>({})), true);
  ps::Status status = udf->Run(ctx);
  EXPECT_FALSE(status.IsOk());
  EXPECT_EQ(status.Msg(), "Hash Shape Should not be Scalar");  
  EXPECT_TRUE(ctx->SetData(1, new WrapperData<TensorShape>(std::vector<size_t>({4, 8})), true).IsOk());
  EXPECT_TRUE(udf->Run(ctx).IsOk());
  EXPECT_TRUE(udf->Run(ctx).IsOk());
  Variable* var;
  EXPECT_TRUE(storage_manager->Get("123", &var).IsOk());
  EXPECT_EQ(DataType::kInt8, var->GetData()->Type());
  EXPECT_EQ(0x0101010101010101l, *var->GetData()->Raw<int64_t>(0));
  EXPECT_EQ(0x0101010101010101l, *var->GetData()->Raw<int64_t>(1));
  EXPECT_EQ(0x0101010101010101l, *var->GetData()->Raw<int64_t>(2));
  EXPECT_EQ(0x0101010101010101l, *var->GetData()->Raw<int64_t>(3));
  EXPECT_NE(nullptr, dynamic_cast<WrapperData<std::unique_ptr<HashMap> >*>(var->GetSlicer()));
  delete ctx;
  delete udf;
}

