contract C {
    function f() external pure {
        assembly "evmasm" ("memory-safe") {}
        assembly { mstore(0,0) }
    }
}
// ----
// :C(creation) true
// :C(runtime) false
