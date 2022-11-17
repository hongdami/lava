// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
// See: https://spdx.org/licenses/

#include <message_infrastructure/csrc/channel/grpc/grpc_port.h>
#include <message_infrastructure/csrc/core/message_infrastructure_logging.h>
#include <message_infrastructure/csrc/channel/grpc/grpc.h>

namespace message_infrastructure {

namespace {

MetaDataPtr GrpcMetaData2MetaData(GrpcMetaDataPtr grpcdata) {
  MetaDataPtr metadata = std::make_shared<MetaData>();
  metadata->nd = grpcdata->nd();
  metadata->type = grpcdata->type();
  metadata->elsize = grpcdata->elsize();
  metadata->total_size = grpcdata->total_size();
  void* data = std::calloc(metadata->elsize * metadata->total_size, 1);
  if (data == nullptr) {
    LAVA_LOG_FATAL("Memory alloc failed, errno: %d\n", errno);
  }
  for (int i = 0; i < metadata->nd; i++) {
    metadata->dims[i] = grpcdata->dims(i);
    metadata->strides[i] = grpcdata->strides(i);
  }
  std::memcpy(data,
              grpcdata->value().c_str(),
              metadata->elsize * metadata->total_size);
  metadata->mdata = data;
  return metadata;
}

}  // namespace

template<>
void RecvQueue<GrpcMetaDataPtr>::FreeData(GrpcMetaDataPtr data)
{}

GrpcChannelServerImpl::GrpcChannelServerImpl(const std::string& name,
                                             const size_t &size)
  :name_(name), size_(size), done_(false) {
  recv_queue_ = std::make_shared<RecvQueue<GrpcMetaDataPtr>>(name_, size_);
}

Status GrpcChannelServerImpl::RecvArrayData(ServerContext* context,
                                            const GrpcMetaData *request,
                                            DataReply* reply) {
  bool rep = true;
  while (recv_queue_->AvailableCount() <=0) {
    helper::Sleep();
    if (done_) {
      rep = false;
      return Status::OK;
    }
  }
  recv_queue_->Push(std::make_shared<GrpcMetaData>(*request));
  reply->set_ack(rep);
  return Status::OK;
}

GrpcMetaDataPtr GrpcChannelServerImpl::Pop(bool block) {
  return recv_queue_->Pop(block);
}

GrpcMetaDataPtr GrpcChannelServerImpl::Front() {
  return recv_queue_->Front();
}

bool GrpcChannelServerImpl::Probe() {
  return recv_queue_->Probe();
}

void GrpcChannelServerImpl::Stop() {
  done_ = true;
  recv_queue_->Stop();
}

GrpcRecvPort::GrpcRecvPort(const std::string& name,
                           const size_t &size,
                           const std::string& url)
  :name_(name), size_(size), done_(false), url_(url) {
  service_ptr_ = std::make_shared<GrpcChannelServerImpl>(name_, size_);
}

void GrpcRecvPort::Start() {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  builder_.AddListeningPort(url_, grpc::InsecureServerCredentials());
  builder_.RegisterService(service_ptr_.get());
  server_ = builder_.BuildAndStart();
}

MetaDataPtr GrpcRecvPort::Recv() {
  GrpcMetaDataPtr recv_data = service_ptr_->Pop(true);
  return GrpcMetaData2MetaData(recv_data);
}

MetaDataPtr GrpcRecvPort::Peek() {
  GrpcMetaDataPtr peek_data = service_ptr_->Front();
  return GrpcMetaData2MetaData(peek_data);
}

void GrpcRecvPort::Join() {
  if (!done_) {
    done_ = true;
    service_ptr_->Stop();
    server_->Shutdown();
  }
}

bool GrpcRecvPort::Probe() {
  return service_ptr_->Probe();
}

void GrpcSendPort::Start() {
  channel_ = grpc::CreateChannel(url_, grpc::InsecureChannelCredentials());
  stub_ = GrpcChannelServer::NewStub(channel_);
}

void GrpcSendPort::Send(DataPtr grpcdata) {
  GrpcMetaData* data = reinterpret_cast<GrpcMetaData*>(grpcdata.get());
  DataReply reply;
  ClientContext context;
  context.set_wait_for_ready(true);
  Status status = stub_->RecvArrayData(&context, *data, &reply);
  if (!reply.ack()) {
    LAVA_LOG_ERR("Send fail!\n");
  }
}

bool GrpcSendPort::Probe() {
  return false;
}

void GrpcSendPort::Join() {
  done_ = true;
}

}  // namespace message_infrastructure
