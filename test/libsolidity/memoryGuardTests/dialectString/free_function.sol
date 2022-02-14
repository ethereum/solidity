function safe() pure returns (uint256 x) {
    assembly { x := 42 }
    assembly "evmasm" ("memory-safe") { mstore(0, 0) }
}
function unsafe() pure returns (uint256 x) {
    assembly { pop(mload(0)) }
}
contract C {
    constructor() {
        unsafe();
    }
    function f() public pure {
        safe();
    }
}
contract D {
    constructor() {
        safe();
    }
    function f() public pure {
        unsafe();
    }
}
// ----
// :C(creation) false
// :C(runtime) true
// :D(creation) true
// :D(runtime) false
