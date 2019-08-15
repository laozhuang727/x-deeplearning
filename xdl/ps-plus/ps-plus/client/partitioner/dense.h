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

#ifndef PS_PLUS_CLIENT_PARTITIONER_DENSE_H_
#define PS_PLUS_CLIENT_PARTITIONER_DENSE_H_

#include "ps-plus/client/partitioner.h"

namespace ps {
namespace client {
namespace partitioner {

class Dense : public Partitioner {
 public:
  virtual Status Split(PartitionerContext* ctx, Data* src, std::vector<Data*>* dst) override;
  virtual Status Combine(PartitionerContext* ctx, Data* src, size_t server_id, std::unique_ptr<Data>* output) override;
  virtual Status CombineInit(PartitionerContext* ctx, std::unique_ptr<Data>* output) override;
};

}
}
}

#endif

