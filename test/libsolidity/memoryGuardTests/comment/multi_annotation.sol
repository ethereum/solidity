contract C {
    constructor() {
        /// @solidity memory-safe-assembly    a memory-safe-assembly
        assembly { mstore(0, 0) }
    }
    function f() internal pure {
        /// @solidity a memory-safe-assembly
        assembly { mstore(0, 0) }
        /// @solidity a
        ///           memory-safe-assembly
        ///           b
        assembly { mstore(0, 0) }
    }
}
// ----
// :C(creation) true
// :C(runtime) true
