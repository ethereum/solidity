## Intro

[oss-fuzz][1] is Google's fuzzing infrastructure that performs continuous fuzzing. What this means is that, each and every upstream commit is automatically fetched by the infrastructure and fuzzed.

## What does this directory contain?

To help oss-fuzz do this, we (as project maintainers) need to provide the following:

- test harnesses: C/C++ tests that define the `LLVMFuzzerTestOneInput` API. This determines what is to be fuzz tested.
- build infrastructure: (c)make targets per fuzzing binary. Fuzzing requires coverage and memory instrumentation of the code to be fuzzed.
- configuration files: These are files with the `.options` extension that are parsed by oss-fuzz. The only option that we use currently is the `dictionary` option that asks the fuzzing engines behind oss-fuzz to use the specified dictionary. The specified dictionary happens to be `solidity.dict.`

`solidity.dict` contains Solidity-specific syntactical tokens that are more likely to guide the fuzzer towards generating parseable and varied Solidity input.

To be consistent and aid better evaluation of the utility of the fuzzing dictionary, we stick to the following rules-of-thumb:
  - Full tokens such as `block.number` are preceded and followed by a whitespace
  - Incomplete tokens including function calls such as `msg.sender.send()` are abbreviated `.send(` to provide some leeway to the fuzzer to sythesize variants such as `address(this).send()`
  - Language keywords are suffixed by a whitespace with the exception of those that end a line of code such as `break;` and `continue;`

## What is libFuzzingEngine.a?

`libFuzzingEngine.a` is an oss-fuzz-related dependency. It is present in the Dockerized environment in which Solidity's oss-fuzz code will be built.

## Is this directory relevant for routine Solidity CI builds?

No. This is the reason why the `add_subdirectory(ossfuzz)` cmake directive is nested under the `if (OSSFUZZ)` predicate. `OSSFUZZ` is a solidity-wide cmake option that is invoked by the ossfuzz solidity-builder-bot in order to compile solidity fuzzer binaries.

[1]: https://github.com/google/oss-fuzz
