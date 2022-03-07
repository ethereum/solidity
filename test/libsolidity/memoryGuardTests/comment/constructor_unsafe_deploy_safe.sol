contract C {
    constructor() {
        uint256 x;
        assembly { x := 0 }
        f();
    }
    function f() internal pure {
        assembly { mstore(0, 0) }
    }
    function g() public pure {
        /// @solidity memory-safe-assembly
        assembly { mstore(0, 0) }
    }
}
// ----
// :C(creation) false
// :C(runtime) true
