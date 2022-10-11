// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
// See: https://spdx.org/licenses/

#include "socket.h"

namespace message_infrastructure {

SharedSocket::SharedSocket(const size_t &mem_size, int socket[2], const int &key) {
//   shmfd_ = shmfd;
  socket_[0] = socket[0];
  socket_[1] = socket[1];
  size_ = mem_size;
  req_name_ += std::to_string(key);
  ack_name_ += std::to_string(key);
}

SharedSocket::SharedSocket(const size_t &mem_size, int socket[2]) {
  socket_[0] = socket[0];
  socket_[1] = socket[1];
  size_ = mem_size;
}

void SharedSocket::InitSemaphore() {
  req_ = sem_open(req_name_.c_str(), O_CREAT, 0644, 0);
  ack_ = sem_open(ack_name_.c_str(), O_CREAT, 0644, 0);
}

void SharedSocket::Start() {
  // RecvPort will post init sem.
}

void SharedSocket::Store(HandleFn store_fn) {
  
  long long buffer[(size_+1)/8];
  store_fn((void*)buffer);
  char temp;
  // sem_wait(ack_);
  LAVA_LOG_ERR("prewrite data.\n");
  size_t length = write(socket_[0], (char *)buffer, size_);
  LAVA_LOG_ERR("postwrite data.\n");
  // sem_post(req_);
  if (length == -1){
    LAVA_LOG_ERR("Write socket failed.\n");
    exit(-1);
  } else if (length != size_){
    LAVA_LOG_ERR("Write socket error, expected size: %zd, got size: %zd", size_, length);
    exit(-1);
  }
  LAVA_LOG_ERR("preread ack.\n");
  length = read(socket_[0], &temp, 1);
  LAVA_LOG_ERR("postread ack.\n");
  if (length != 1 || temp != 'a'){

  }
//   LAVA_LOG_ERR("Write socket size: %zd.\n", length);
//   for(int i=0;i<(size_+1)/8;i++){
//     LAVA_LOG_ERR("Write Socket Buffer: %lld\n", buffer[i]);
//   }
}

bool SharedSocket::Load(HandleFn consume_fn) {
  long long buffer[(size_+1)/8];
  bool ret = false;
  int val;
  char temp = 'a';
  size_t length = 0;
  // if (!sem_trywait(req_))
  // {
      LAVA_LOG_ERR("preread data.\n");
      length = read(socket_[1], (char *)buffer, size_);
      LAVA_LOG_ERR("postread data.\n");
    //   consume_fn(MemMap());
      ret = true;
  // }
  // sem_getvalue(ack_, &val);
  // if (val == 0) {
  //   sem_post(ack_);
  // }
  if (!ret){
    return ret;
  }
  if (length < 0) {
    ret = false;
    LAVA_LOG_ERR("Read socket failed.");
    exit(-1);
  } else if (size_ != length) {
    ret = false;
    LAVA_LOG_ERR("Read socket error, expected size: %zd, got size: %zd", size_, length);
    exit(-1);
  }
  consume_fn((void*)buffer);
//   LAVA_LOG_ERR("Read socket size: %zd.\n", length);
//   for(int i=0;i<(size_+1)/8;i++){
//     LAVA_LOG_ERR("Read Socket Buffer: %lld\n", buffer[i]);
//   }
  LAVA_LOG_ERR("prewrite ack.\n");
  length = write(socket_[1], &temp, 1);
  LAVA_LOG_ERR("postwrite ack.\n");
  if (length != 1){

  }
  return ret;
}

void SharedSocket::Close() {
  sem_close(req_);
  sem_close(ack_);
}

void* SharedSocket::MemMap() {
//   return (data_ = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_, 0));
    return NULL;
}


// int SharedSocket::GetDataElem(int offset) {
//   return static_cast<int> (*(((char*)data_) + offset));
// }

SharedSocket::~SharedSocket() {
  Close();
  sem_unlink(req_name_.c_str());
  sem_unlink(ack_name_.c_str());
}

// RwSharedSocket::RwSharedSocket(const size_t &mem_size, const int &shmfd, const int &key)
//   : size_(mem_size), shmfd_(shmfd)
// {
//   sem_name_ += std::to_string(key);
// }

// void RwSharedSocket::InitSemaphore() {
//   sem_ = sem_open(sem_name_.c_str(), O_CREAT, 0644, 0);
// }

// void RwSharedSocket::Start() {
//   sem_post(sem_);
// }

// void RwSharedSocket::Handle(HandleFn handle_fn) {
//   sem_wait(sem_);
//   handle_fn(GetData());
//   sem_post(sem_);
// }

// void RwSharedSocket::Close() {
//   sem_close(sem_);
// }

// void* RwSharedSocket::GetData() {
//   return (data_ = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_, 0));
// }

// RwSharedSocket::~RwSharedSocket() {
//   Close();
//   sem_unlink(sem_name_.c_str());
// }

void SharedSktManager::DeleteSharedSocket(int &socket) {
    if (sockets_.find(socket) != sockets_.end()) {
        close(socket);
        sockets_.erase(socket);
    } else {
        LAVA_LOG_WARN(LOG_SSKP,"There is no socket whose fd is %d.\n", socket);
    }
  // Release specific shared memory
//   if (shm_strs_.find(shm_str) != shm_strs_.end()) {
//     shm_unlink(shm_str.c_str());
//     shm_strs_.erase(shm_str);
//   } else {
//     LAVA_LOG_WARN(LOG_SMMP,"There is no shmem whose name is %s.\n", shm_str.c_str());
//   }
}

SharedSktManager::~SharedSktManager() {
  int result = 0;
  for (auto it = sockets_.begin(); it != sockets_.end(); it++) {
    close(*it);
  }
  sockets_.clear();
}

SharedSktManager SharedSktManager::ssm_;

SharedSktManager& GetSharedSktManager() {
  SharedSktManager &ssm = SharedSktManager::ssm_;
  return ssm;
}
}  // namespace message_infrastructure
