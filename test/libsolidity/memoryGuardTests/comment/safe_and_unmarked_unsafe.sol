contract C {
    function f() external pure {
        /// @solidity memory-safe-assembly
        assembly {}
        assembly { mstore(0,0) }
    }
}
// ----
// :C(creation) true
// :C(runtime) false
