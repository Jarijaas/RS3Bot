#pragma once

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include <functional>
#include "proto\world.grpc.pb.h"

class RPCServer;

enum class CompletionState { WAITING, COMPLETED };

/*
Parent class required, because  casting classes
with different template variables to a common base class,
so that the Proceed function is common.
*/
class RequestHandlerBase {
 public:
  virtual void Proceed(){};

  // Let's implement a tiny state machine with the following states.
  enum CallStatus { CREATE, BUSY, PROCESS, FINISH };

  CompletionState State() { return state_; }

  virtual bool IsStreamingRequest() { return false; }

 protected:
  CallStatus status_;  // The current serving state.
  CompletionState state_;
};

template <typename TRequest, typename TResponse = Empty>
class RequestHandler : RequestHandlerBase {
 public:
  // Take in the "service" instance (in this case representing an asynchronous
  // server) and the completion queue "cq" used for asynchronous communication
  // with the gRPC runtime.

  typedef std::function<void(
      RS3::AsyncService*, grpc::ServerContext*, TRequest*,
      grpc::ServerAsyncResponseWriter<TResponse>*, grpc::CompletionQueue*,
      grpc::ServerCompletionQueue*, void*)>
      Processor;

  typedef std::function<grpc::Status(const TRequest& req, TResponse*)> Handler;

  RequestHandler(RS3::AsyncService* service, grpc::ServerCompletionQueue* cq,
                 const Processor& processor, const Handler& handler)
      : service_(service),
        cq_(cq),
        responder_(&ctx_),
        processor_(processor),
        handler_(handler) {
    status_ = CREATE;
    state_ = CompletionState::COMPLETED;

    // Invoke the serving logic right away.
    Proceed();
  }

  typedef void(__cdecl* _Proceed)(void);

  void Proceed() {
    if (status_ == CREATE) {
      // Make this instance progress to the PROCESS state.
      status_ = PROCESS;

      // As part of the initial CREATE state, we *request* that the system
      // start processing SayHello requests. In this request, "this" acts are
      // the tag uniquely identifying the request (so that different CallData
      // instances can serve different requests concurrently), in this case
      // the memory address of this CallData instance.

      processor_(service_, &ctx_, &request_, &responder_, cq_, cq_, this);
    } else if (status_ == PROCESS) {
      // Spawn a new CallData instance to serve new clients while we process
      // the one for this CallData. The instance will deallocate itself as
      // part of its FINISH state.

      new RequestHandler(service_, cq_, processor_, handler_);

      responder_.Finish(response_, handler_(request_, &response_), this);

      // And we are done! Let the gRPC runtime know we've finished, using the
      // memory address of this instance as the uniquely identifying tag for
      // the event.
      status_ = FINISH;
    } else {
      GPR_ASSERT(status_ == FINISH);
      // Once in the FINISH state, deallocate ourselves (CallData).
      delete this;
    }
  }

 private:
  // The means of communication with the gRPC runtime for an asynchronous
  // server.
  RS3::AsyncService* service_;

  // The producer-consumer queue where for asynchronous server notifications.
  grpc::ServerCompletionQueue* cq_;

  // Context for the rpc, allowing to tweak aspects of it such as the use
  // of compression, authentication, as well as to send metadata back to the
  // client.
  grpc::ServerContext ctx_;

  TRequest request_;
  TResponse response_;

  // The means to get back to the client.
  grpc::ServerAsyncResponseWriter<TResponse> responder_;

  const Processor processor_;
  const Handler handler_;
};

template <typename TRequest, typename TResponse = Empty>
class StreamRequestHandler : RequestHandlerBase {
 public:
  // Take in the "service" instance (in this case representing an asynchronous
  // server) and the completion queue "cq" used for asynchronous communication
  // with the gRPC runtime.

  typedef std::function<void(RS3::AsyncService*, grpc::ServerContext*,
                             TRequest*, grpc::ServerAsyncWriter<TResponse>*,
                             grpc::CompletionQueue*,
                             grpc::ServerCompletionQueue*, void*)>
      Processor;

  typedef std::function<CompletionState(
      RPCServer* server, const TRequest& req,
      StreamRequestHandler<TRequest, TResponse>*)>
      Handler;

  StreamRequestHandler(RPCServer* server, RS3::AsyncService* service,
                       grpc::ServerCompletionQueue* cq,
                       const Processor& processor, const Handler& handler)
      : server_(server),
        service_(service),
        cq_(cq),
        responder_(&ctx_),
        processor_(processor),
        handler_(handler) {
    status_ = CREATE;
    state_ = CompletionState::COMPLETED;

    // Invoke the serving logic right away.
    Proceed();
  }

  bool IsStreamingRequest() override { return true; }

  typedef void(__cdecl* _Proceed)(void);

  void Proceed() {
    if (status_ == CREATE) {
      // Make this instance progress to the BUSY state.
      status_ = BUSY;

      // As part of the initial CREATE state, we *request* that the system
      // start processing requests. In this request, "this" acts are
      // the tag uniquely identifying the request (so that different CallData
      // instances can serve different requests concurrently), in this case
      // the memory address of this CallData instance.

      processor_(service_, &ctx_, &request_, &responder_, cq_, cq_, this);
    } else if (status_ == BUSY) {
      status_ = PROCESS;

      state_ = handler_(server_, request_, this);

      // This handler is now busy, create a new one to handle new requests
      new StreamRequestHandler(server_, service_, cq_, processor_, handler_);
    } else if (status_ == PROCESS) {
      // responder_.Finish(handler_(request_, this), this);
      state_ = handler_(server_, request_, this);
    } else {
      GPR_ASSERT(status_ == FINISH);
      // Once in the FINISH state, deallocate ourselves (CallData).
      delete this;
    }
  }

  void Write(const TResponse& msg) { responder_.Write(msg, this); }

  void Finish(const grpc::Status& status) {
    responder_.Finish(status, this);

    // And we are done! Let the gRPC runtime know we've finished, using the
    // memory address of this instance as the uniquely identifying tag for
    // the event.
    status_ = FINISH;
  }

 private:
  // The means of communication with the gRPC runtime for an asynchronous
  // server.
  RS3::AsyncService* service_;

  // The producer-consumer queue where for asynchronous server notifications.
  grpc::ServerCompletionQueue* cq_;

  // Context for the rpc, allowing to tweak aspects of it such as the use
  // of compression, authentication, as well as to send metadata back to the
  // client.
  grpc::ServerContext ctx_;

  TRequest request_;

  // The means to get back to the client.
  grpc::ServerAsyncWriter<TResponse> responder_;

  const Processor processor_;
  const Handler handler_;

  RPCServer* server_;
};