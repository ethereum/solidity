## Intro

[oss-fuzz][1] is Google's fuzzing infrastructure that performs continuous fuzzing. What this means is that, each and every upstream commit is automatically fetched by the infrastructure and fuzzed on a daily basis.

## How to build fuzzers?

We have multiple fuzzers, some based on string input and others on protobuf input. To build them, please do the following:

- Create a local docker image from `Dockerfile.ubuntu.clang.ossfuzz` in the `.circleci/docker` sub-directory. Please note that this step is likely to take at least an hour to complete. Therefore, it is recommended to do it when you are away from the computer (and the computer is plugged to power since we do not want a battery drain).

```
$ cd .circleci/docker
$ docker build -t solidity-ossfuzz-local -f Dockerfile.ubuntu.clang.ossfuzz .
```

- Login to the docker container sourced from the image built in the previous step from the solidity parent directory

```
## Host
$ cd solidity
$ docker run -v `pwd`:/src/solidity -ti solidity-ossfuzz-local /bin/bash
## Docker shell
$ cd /src/solidity
```

- Run cmake and build fuzzer harnesses

```
## Docker shell
$ cd /src/solidity
$ rm -rf fuzzer-build && mkdir fuzzer-build && cd fuzzer-build
## Compile protobuf C++ bindings
$ protoc --proto_path=../test/tools/ossfuzz yulProto.proto --cpp_out=../test/tools/ossfuzz
$ protoc --proto_path=../test/tools/ossfuzz abiV2Proto.proto --cpp_out=../test/tools/ossfuzz
## Run cmake
$ export CC=clang CXX=clang++
$ cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/libfuzzer.cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Release} ..
$ make ossfuzz ossfuzz_proto ossfuzz_abiv2 -j
```

## Why the elaborate docker image to build fuzzers?

For the following reasons:

- Fuzzing binaries **must** link against libc++ and not libstdc++
  - This is [because][2] (1) MemorySanitizer (which flags uses of uninitialized memory) depends on libc++; and (2) because libc++ is instrumented (to check for memory and type errors) and libstdc++ not, the former may find more bugs.

- Linking against libc++ requires us to compile everything solidity depends on from source (and link these against libc++ as well)

- To reproduce the compiler versions used by upstream oss-fuzz bots, we need to reuse their docker image containing the said compiler versions

- Some fuzzers depend on libprotobuf, libprotobuf-mutator, libevmone etc. which may not be available locally; even if they were they might not be the right versions

## What is LIB\_FUZZING\_ENGINE?

oss-fuzz contains multiple fuzzer back-ends i.e., fuzzers. Each back-end may require different linker flags. oss-fuzz builder bot defines the correct linker flags via a bash environment variable called `LIB_FUZZING_ENGINE`.

For the solidity ossfuzz CI build, we use the libFuzzer back-end. This back-end requires us to manually set the `LIB_FUZZING_ENGINE` to `-fsanitize=fuzzer`.

## What does the ossfuzz directory contain?

To help oss-fuzz do this, we (as project maintainers) need to provide the following:

- test harnesses: C/C++ tests that define the `LLVMFuzzerTestOneInput` API. This determines what is to be fuzz tested.
- build infrastructure: (c)make targets per fuzzing binary. Fuzzing requires coverage and memory instrumentation of the code to be fuzzed.
- configuration files: These are files with the `.options` extension that are parsed by oss-fuzz. The only option that we use currently is the `dictionary` option that asks the fuzzing engines behind oss-fuzz to use the specified dictionary. The specified dictionary happens to be `solidity.dict.`

`solidity.dict` contains Solidity-specific syntactical tokens that are more likely to guide the fuzzer towards generating parseable and varied Solidity input.

To be consistent and aid better evaluation of the utility of the fuzzing dictionary, we stick to the following rules-of-thumb:
  - Full tokens such as `block.number` are preceded and followed by a whitespace
  - Incomplete tokens including function calls such as `msg.sender.send()` are abbreviated `.send(` to provide some leeway to the fuzzer to sythesize variants such as `address(this).send()`
  - Language keywords are suffixed by a whitespace with the exception of those that end a line of code such as `break;` and `continue;`

[1]: https://github.com/google/oss-fuzz
[2]: https://github.com/google/oss-fuzz/issues/1114#issuecomment-360660201
