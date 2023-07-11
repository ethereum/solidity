# EVMC

This is an import of [EVMC](https://github.com/ethereum/evmc) version [10.1.0](https://github.com/ethereum/evmc/releases/tag/v10.1.0).

Important: The `MockedAccount.storage` is changed to a `map` from `unordered_map` as ordering is important for fuzzing.
