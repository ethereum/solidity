contract C {
    constructor(uint256 x) {
        assembly { x := 4 }
        /// @solidity memory-safe-assembly
        assembly { mstore(0, 0) }
    }
    function f() public pure {
        assembly { mstore(0,0) }
    }
}
contract D {
    constructor() {
        assembly { mstore(0,0) }
    }
    function f(uint256 x) public pure {
        assembly { x := 4 }
        /// @solidity memory-safe-assembly
        assembly { mstore(0, 0) }
    }
}
// ----
// :C(creation) true
// :C(runtime) false
// :D(creation) false
// :D(runtime) true
