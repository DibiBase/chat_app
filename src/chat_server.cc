#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "chat.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;

class ChatServiceImpl final : public ChatService::Service {
public:
  ChatServiceImpl() { client_num = 0; }

private:
  Status chat(ServerContext *context,
              ServerReaderWriter<ChatMessage, ChatMessage> *stream) override {
    ChatMessage msg;

    while (stream->Read(&msg)) {
      std::cout << msg.id() << " " << msg.from() << ": " << msg.message()
                << "\n";
      if (!streams.count(msg.id()))
        streams.insert({msg.id(), stream});
      send_all(msg);
    }

    return Status::OK;
  }

  Status add(ServerContext *context, const AddMessage *request,
             IdMessage *response) override {
    ChatMessage msg;
    msg.set_from(request->from());
    msg.set_message("has conneted");
    std::cout << msg.from() << ": " << msg.message() << "\n";

    send_all(msg);
    response->set_id(++client_num);
    return Status::OK;
  }

  Status withdraw(ServerContext *context, const ChatMessage *request,
                  Nothing *response) override {
    streams.erase(request->id());
    ChatMessage msg;
    msg.set_from(request->from());
    msg.set_message("has disconneted");
    std::cout << msg.from() << ": " << msg.message() << "\n";

    send_all(msg);
    return Status::OK;
  }

  void send_all(const ChatMessage &msg) {
    std::string message = trim(msg.message());

    if (message == "")
      return;

    for (const auto &stream : streams) {
      if (stream.first != msg.id())
        stream.second->Write(msg);
    }
  }

  std::string trim(std::string message) {
    message.erase(message.begin(), std::find_if(message.begin(), message.end(),
                                                [](unsigned char ch) {
                                                  return !std::isspace(ch);
                                                }));
    message.erase(
        std::find_if(message.rbegin(), message.rend(),
                     [](unsigned char ch) { return !std::isspace(ch); })
            .base(),
        message.end());
    return message;
  }

  int client_num;
  std::map<int, ServerReaderWriter<ChatMessage, ChatMessage> *> streams;
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  ChatServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char **argv) {
  RunServer();

  return 0;
}