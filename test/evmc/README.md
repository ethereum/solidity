# EVMC

This is an import of [EVMC](https://github.com/ethereum/evmc) version [12.0.0](https://github.com/ethereum/evmc/releases/tag/v12.0.0).

Steps when upgrading:
- Copy all from [include/evmc](https://github.com/ethereum/evmc/tree/master/include/evmc) to [test/evmc](https://github.com/ethereum/solidity/tree/develop/test/evmc)
    - Note that you should delete (or not copy in the first place) `tooling.hpp` and `instructions.h`.
- Copy [`loader.c`](https://github.com/ethereum/evmc/blob/master/lib/loader/loader.c) to [test/evmc](https://github.com/ethereum/solidity/tree/develop/test/evmc)
- `MockedAccount.storage` in `mocked_host.hpp` should be changed to a `map` from `unordered_map` as ordering is important for fuzzing. You'll also need to include `<map>`.
    See [PR #11094](https://github.com/ethereum/solidity/pull/11094) for more details.
