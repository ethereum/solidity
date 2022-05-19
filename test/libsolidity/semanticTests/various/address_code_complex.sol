contract A {
    constructor() {
        assembly {
            // This is only 7 bytes here.
            mstore(0, 0x48aa5566000000)
            return(0, 32)
        }
    }
}

contract C {
    function f() public returns (bytes memory) { return address(new A()).code; }
    function g() public returns (uint) { return address(new A()).code.length; }
}
// ----
// f() -> 0x20, 0x20, 0x48aa5566000000
// g() -> 0x20
