// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
// See: https://spdx.org/licenses/

#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_

#include <functional>

namespace message_infrastructure {

using HandleFn = std::function<void(void *)>;

class SharedCommunicator {
 public:
  SharedCommunicator() {}
  virtual void Start() = 0;
  virtual bool Load(HandleFn consume_fn) = 0;
  virtual void Store(HandleFn store_fn) = 0;
};

using SharedCommunicatorPtr = std::shared_ptr<SharedCommunicator>;

}  // namespace message_infrastructure

#endif  // COMMUNICATOR_H_