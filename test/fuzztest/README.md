# Property-based / fuzz tests

This directory holds **property-based / fuzz tests** built
on [Google FuzzTest](https://github.com/google/fuzztest) (vendored at `deps/fuzztest`).
Instead of asserting a fixed output for a fixed input, a property test states a property that must hold for all inputs
from a domain, and FuzzTest searches that domain for counterexamples.

## Options

Two CMake options control this (defined in `cmake/EthOptions.cmake`):

- `PROPERTY_BASED_TESTS` (default `OFF`) — master on/off switch. While it is `OFF`, `deps/fuzztest` is never added 
  to the build, so none of its transitive `FetchContent` dependencies are configured.
- `PROPERTY_BASED_TESTS_MODE` (default `unittest`, values `unittest` / `fuzzing`). Selects the run mode. Only
  meaningful when `PROPERTY_BASED_TESTS` is `ON`.
  - `unittest`: each `FUZZ_TEST` runs as an ordinary test case with a 1sec burst of random inputs.
  - `fuzzing`: coverage-guided fuzzing. Requires Clang.

## Configure

```sh
# unit-test mode (works with the default g++ build)
cmake -S . -B build-prop -G Ninja -DPROPERTY_BASED_TESTS=ON

# fuzzing mode (Clang required)
CC=clang CXX=clang++ cmake -S . -B build-fuzz -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/fuzztest.cmake
```

## evmone

`solidity_fuzztest` compiles and runs contracts, so it needs `libevmone.so` to be loadable. It is picked up from the
default library search path, or from `$ETH_EVMONE` if that is set:

```sh
LD_LIBRARY_PATH=/path/to/evmone/lib build/test/fuzztest/solidity_fuzztest
```
