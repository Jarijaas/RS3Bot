npx pbjs -t static-module -w commonjs --js ../js_proto/generated.js ../protobuf/world.proto
node ./node_modules/protobufjs/bin/pbts -o ../js_proto/generated.d.ts ../js_proto/generated.js