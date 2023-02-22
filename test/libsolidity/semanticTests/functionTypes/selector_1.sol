contract B {
    function ext() external {}
    function pub() public {}
}

contract C is B {
    function test() public returns (bytes4, bytes4, bytes4, bytes4) {
        return (B.ext.selector, B.pub.selector, this.ext.selector, pub.selector);
    }
}
// ====
// compileViaYul: true
// ----
// test() -> 0xcf9f23b500000000000000000000000000000000000000000000000000000000, 0x7defb41000000000000000000000000000000000000000000000000000000000, 0xcf9f23b500000000000000000000000000000000000000000000000000000000, 0x7defb41000000000000000000000000000000000000000000000000000000000
