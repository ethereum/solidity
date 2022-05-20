contract test {
    function boo() public returns (bytes4 value) {
        return msg.sig;
    }

    function foo(uint256 a) public returns (bytes4 value) {
        return boo();
    }
}
// ====
// compileToEwasm: also
// ----
// foo(uint256): 0x0 -> 0x2fbebd3800000000000000000000000000000000000000000000000000000000
