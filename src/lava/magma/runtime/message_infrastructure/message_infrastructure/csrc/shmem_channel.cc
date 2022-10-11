// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
// See: https://spdx.org/licenses/

#include "shmem_channel.h"
#include "port_proxy.h"
#include "utils.h"

namespace message_infrastructure {

ShmemChannel::ShmemChannel(const ChannelType &channel_type,
                           const std::string &src_name,
                           const std::string &dst_name,
                           const size_t &size,
                           const size_t &nbytes) {
  unsigned long shmem_size = nbytes + sizeof(MetaData);
  size_t items_size = size;
  switch (channel_type) {
    case RPCCHANNEL:
      break;
    case DDSCHANNEL:
      break;
    case SOCKETCHANNEL:
      items_size = 0;
      shm_ = GetSharedSktManager().AllocChannelSharedSocket<SharedSocket>(shmem_size);
      break;
    default:
      shm_ = GetSharedMemManager().AllocChannelSharedMemory<SharedMemory>(shmem_size);
  }
  // shm_ = GetSharedMemManager().AllocChannelSharedMemory<SharedMemory>(shmem_size);

  send_port_ = std::make_shared<ShmemSendPort>(src_name, shm_, items_size, shmem_size);
  recv_port_ = std::make_shared<ShmemRecvPort>(dst_name, shm_, items_size, shmem_size);
}

AbstractSendPortPtr ShmemChannel::GetSendPort() {
  return send_port_;
}

AbstractRecvPortPtr ShmemChannel::GetRecvPort() {
  return recv_port_;
}

std::shared_ptr<ShmemChannel> GetShmemChannel(const ChannelType &channel_type,
                              const size_t &size,
                              const size_t &nbytes,
                              const std::string &src_name,
                              const std::string &dst_name) {
  return (std::make_shared<ShmemChannel>(channel_type,
                                         src_name,
                                         dst_name,
                                         size,
                                         nbytes));
}
}  // namespace message_infrastructure
