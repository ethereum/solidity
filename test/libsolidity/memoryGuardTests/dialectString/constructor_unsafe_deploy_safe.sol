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
        assembly "evmasm" ("memory-safe") { mstore(0, 0) }
    }
}
// ----
// :C(creation) false
// :C(runtime) true
