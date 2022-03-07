contract C {
    function f() external pure {
        assembly "evmasm" ("memory-safe") {}
        assembly {}
    }
}
// ----
// :C(creation) true
// :C(runtime) true
