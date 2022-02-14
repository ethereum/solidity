contract C {
    function f() external pure {
        /// @solidity memory-safe-assembly
        assembly {}
        assembly {}
    }
}
// ----
// :C(creation) true
// :C(runtime) true
