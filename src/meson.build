protoc = find_program('protoc')
grpc_cpp = find_program('grpc_cpp_plugin')

threadlibs = dependency('threads')
protobuf = dependency('protobuf')
grpc = dependency('grpc', required: false)
if not grpc.found()
  grpc = cppc.find_library('grpc')
endif

grpcpp = dependency('grpc++', required: false)
if not grpcpp.found()
  grpcpp = cppc.find_library('grpc++')
endif

grpc_reflection = cppc.find_library('grpc++_reflection', required: false)

if meson.version().version_compare('>=0.56.0')
  project_source_root = meson.project_source_root()
else
  project_source_root = meson.source_root()
endif


protoc_gen = generator(protoc,
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=' + project_source_root + '/proto',
    '--cpp_out=@BUILD_DIR@',
    '@INPUT@'])

grpc_gen = generator(protoc,
  output    : ['@BASENAME@.grpc.pb.cc', '@BASENAME@.grpc.pb.h'],
  arguments : ['--proto_path=' + project_source_root + '/proto',
    '--grpc_out=@BUILD_DIR@',
    '--plugin=protoc-gen-grpc=' + grpc_cpp.path(),
    '@INPUT@'])

src_pb = protoc_gen.process(
  project_source_root + '/proto/chat.proto',
  preserve_path_from : project_source_root + '/proto')

src_grpc = grpc_gen.process(
  project_source_root + '/proto/chat.proto',
  preserve_path_from : project_source_root + '/proto')

dep_grpc = [
  grpc,
  grpc_reflection,
  grpcpp,
  protobuf,
  threadlibs,
]

executable('chat_client',
  sources: ['chat_client.cc', src_pb, src_grpc],
  dependencies: dep_grpc,
  install: true,
)

executable('chat_server',
  sources: ['chat_server.cc', src_pb, src_grpc],
  dependencies: dep_grpc,
  install: true,
)