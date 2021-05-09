contract B {
    function ext() external {}
    function pub() public {}
}

contract D {
    function test() public returns (bytes4, bytes4) {
        return (B.ext.selector, B.pub.selector);
    }
}
// ====
// compileViaYul: true
// compileToEwasm: also
// ----
// test() -> 0xcf9f23b500000000000000000000000000000000000000000000000000000000, 0x7defb41000000000000000000000000000000000000000000000000000000000
