// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
// See: https://spdx.org/licenses/

#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <memory>
#include <set>
#include <string>
#include <atomic>
#include <functional>
#include <cstdlib>
#include <ctime>

#include "message_infrastructure_logging.h"
#include "communicator.h"

namespace message_infrastructure {

// #define SHM_FLAG O_RDWR | O_CREAT
// #define SHM_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

// using HandleFn = std::function<void(void *)>;

class SharedSocket : public SharedCommunicator{
 public:
  SharedSocket() {}
  SharedSocket(const size_t &mem_size, int socket[2], const int &key);
  SharedSocket(const size_t &mem_size, int socket[2]);
  ~SharedSocket();
  void Start();
  bool Load(HandleFn consume_fn);
  void Store(HandleFn store_fn);
  void Close();
  void InitSemaphore();
//   int GetDataElem(int offset);

 private:
  size_t size_;
//   int shmfd_;
  int socket_[2];
  std::string req_name_ = "req";
  std::string ack_name_ = "ack";
  sem_t *req_;
  sem_t *ack_;
  void *data_;

  void* MemMap();
};

// class RwSharedSocket {
//  public:
//   RwSharedSocket(const size_t &mem_size, const int &shmfd, const int &key);
//   ~RwSharedSocket();
//   void InitSemaphore();
//   void Start();
//   void Handle(HandleFn handle_fn);
//   void Close();

//  private:
//   size_t size_;
//   int shmfd_;
//   std::string sem_name_ = "sem";
//   sem_t *sem_;
//   void *data_;

//   void *GetData();
// };

using SharedSocketPtr = std::shared_ptr<SharedSocket>;
// using RwSharedSocketPtr = std::shared_ptr<RwSharedSocket>;

class SharedSktManager {
 public:
  ~SharedSktManager();

  template<typename T>
  std::shared_ptr<T> AllocChannelSharedSocket(const size_t &mem_size) {
    int socket[2];
    int random = std::rand();
    int err = socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, socket);
    if (err == -1){
        LAVA_LOG_ERR("Create shared socket object failed.\n");
        exit(-1);
    }
    sockets_.insert(socket[0]);
    sockets_.insert(socket[1]);
    std::shared_ptr<T> sharedSocket = std::make_shared<T>(mem_size, socket, random);
    sharedSocket->InitSemaphore();
    return sharedSocket;

    // int random = std::rand();
    // std::string str = shm_str_ + std::to_string(random);
    // int shmfd = shm_open(str.c_str(), SHM_FLAG, SHM_MODE);
    // if (shmfd == -1) {
    //   LAVA_LOG_ERR("Create shared memory object failed.\n");
    //   exit(-1);
    // }
    // int err = ftruncate(shmfd, mem_size);
    // if (err == -1) {
    //   LAVA_LOG_ERR("Resize shared memory segment failed.\n");
    //   exit(-1);
    // }
    // shm_strs_.insert(str);
    // std::shared_ptr<T> shm = std::make_shared<T>(mem_size, shmfd, random);
    // shm->InitSemaphore();
    // return shm;
  }

  void DeleteSharedSocket(int &socket);
  friend SharedSktManager &GetSharedSktManager();

 private:
  SharedSktManager() {
    std::srand(std::time(nullptr));
  }
  std::set<std::int32_t> sockets_;
  static SharedSktManager ssm_;
//   std::string shm_str_ = "shm";
};

SharedSktManager& GetSharedSktManager();

using SharedSktManagerPtr = std::shared_ptr<SharedSktManager>;

}  // namespace message_infrastructure

#endif  // SOCKET_H_
