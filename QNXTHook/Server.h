#pragma once


#include <grpc++/grpc++.h>
#include <queue>
#include <unordered_map>
#include <vector>
#include "proto\world.grpc.pb.h"
#include "datacall.h"

using grpc::ServerCompletionQueue;

class RPCServer {
public:
  RPCServer();
  ~RPCServer();
  void Run();
  void Handle();

  template <typename T> void PushEvent(const T &event) {
    std::lock_guard<std::mutex> guard(eventsMutex_);
    const std::type_info &ti = typeid(T);
    events_[ti.hash_code()].push(std::make_shared<T>(event));
  }

  template <typename T>
  const std::queue<std::shared_ptr<google::protobuf::Message>> &GetEvents() {
    const std::type_info &ti = typeid(T);
    return events_[ti.hash_code()];
  }


  template <typename TResponse>
  void NewGenericStreamConsumer(const std::function<void(RS3::AsyncService*, grpc::ServerContext*,
	  Empty*, grpc::ServerAsyncWriter<TResponse>*,
	  grpc::CompletionQueue*,
	  grpc::ServerCompletionQueue*, void*)>& processor) {

	  new StreamRequestHandler<Empty, TResponse>(
		  this, &asyncService_, cq_.get(),
		  processor,
		  [](RPCServer *server, const Empty &req,
			  StreamRequestHandler<Empty, TResponse> *writer) -> CompletionState {

		  auto msgs = server->GetEvents<TResponse>();
		  if (!msgs.empty()) {
			  writer->Write(*std::static_pointer_cast<TResponse>(msgs.front()));
			  return CompletionState::COMPLETED;
		  }

		  return CompletionState::WAITING;
	  });
  }

private:
  RS3::Service worldService_;
  RS3::AsyncService asyncService_;

  std::unique_ptr<ServerCompletionQueue,
                  std::default_delete<ServerCompletionQueue>> cq_;
  std::unique_ptr<grpc::Server> grpcServer_;

  std::unordered_map<size_t,
                     std::queue<std::shared_ptr<google::protobuf::Message>>>
      events_;
  std::mutex eventsMutex_;

  std::vector<RequestHandlerBase *> waitingStreams_;
};