# EVMC

This is an import of [EVMC](https://github.com/ipsilon/evmc) as vendored in
[evmone 0.25.0](https://github.com/ipsilon/evmone/tree/v0.25.0/evmc) (ABI version 19).

The standalone `ipsilon/evmc` repository stopped cutting releases after
[v12.1.0](https://github.com/ipsilon/evmc/releases/tag/v12.1.0) (ABI 12) and has since been archived.
EVMC development continues inside the `evmc/` subdirectory of the `evmone` repository instead, so later
ABI versions (13 onward, including the current ABI 19) are only available from an `evmone` checkout or
release, not from a versioned `evmc` release.

Steps when upgrading:
- Copy all from [evmc/include/evmc](https://github.com/ipsilon/evmone/tree/v0.25.0/evmc/include/evmc) to [test/evmc](https://github.com/argotorg/solidity/tree/develop/test/evmc)
    - As of evmone 0.23.0 that directory no longer contains `tooling.hpp` or `instructions.h` (an
      older instruction to delete/skip them on copy no longer applies — there is nothing there to
      skip). If a future upstream reintroduces files like these that don't belong in this vendored
      copy, drop them then.
    - Since ABI 19 `evmc.h` declares `struct evmc_state_gas` with C++ default member initializers
      (`int64_t left = 0;`, `int64_t spilled = 0;`), which are not valid C. Remove the two `= 0`
      after copying, because `loader.c` includes `evmc.h` and is compiled as C.
- `loader.c` / `loader.h` do **not** need to be refreshed, and in fact can no longer be, from an
  `evmone` checkout: as of evmone 0.23.0, `evmc/lib/` only builds the `evmc`, `evmc_cpp` and
  `mocked_host` CMake targets — the EVMC loader is gone. The copies kept in this directory are
  ABI-agnostic (they only read `vm->abi_version` and compare it against the compile-time
  `EVMC_ABI_VERSION`), so they need no per-version maintenance and can be left untouched on future
  upgrades.
- `mocked_host.hpp` is copied unchanged. `MockedAccount.storage` is an `unordered_map`, so
  `EVMHost::get_address_storage()` returns a sorted copy, because ordering is important for
  fuzzing (see [PR #11094](https://github.com/argotorg/solidity/pull/11094)).
