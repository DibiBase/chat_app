#include <algorithm>
#include <future>
#include <grpcpp/impl/codegen/sync_stream.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "chat.grpc.pb.h"
#include "chat.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Status;

class ChatClient {
public:
  ChatClient(std::shared_ptr<Channel> channel)
      : stub_(ChatService::NewStub(channel)) {}

  void add(const std::string &user) {
    this->user = user;
    
    AddMessage request;
    request.set_from(user);

    ClientContext context;
    IdMessage replay;
    Status status = stub_->add(&context, request, &replay);
    id = replay.id();
  }

  void withdraw() {
    ChatMessage request;
    request.set_id(id);
    request.set_from(user);

    Nothing replay;

    ClientContext context;
    Status status = stub_->withdraw(&context, request, &replay);
  }

  void chat() {
    ChatMessage request;

    ClientContext context;
    stream = stub_->chat(&context);

    std::thread send_thread(&ChatClient::send, this);
    std::thread receive_thread(&ChatClient::receive, this);

    send_thread.join();
    receive_thread.join();
  }

  void send() {
    ChatMessage request;
    std::string msg;

    while (std::getline(std::cin, msg)) {
      request.set_id(id);
      request.set_from(user);
      request.set_message(msg);
      stream->Write(request);
    }
  }

  void receive() {
    ChatMessage request;
    while (stream->Read(&request)) {
      std::cout << request.from() << ": " << request.message() << "\n";
    }
  }

private:
  int id;
  std::string user;
  std::unique_ptr<ChatService::Stub> stub_;
  std::unique_ptr<grpc::ClientReaderWriter<ChatMessage, ChatMessage>> stream;
};

ChatClient *chater;

void exit_handler(int signum) {
  std::cout << "\b\bKeyboardInterrupt\n";

  chater->withdraw();

  exit(signum);
}

int main(int argc, char **argv) {
  signal(SIGINT, exit_handler);
  std::string target_str = "localhost:50051";

  chater = new ChatClient(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  std::string user;
  std::cout << "Enter your name: ";
  std::cin >> user;
  chater->add(user);
  chater->chat();

  return 0;
}