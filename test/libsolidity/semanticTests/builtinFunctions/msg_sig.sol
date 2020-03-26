contract test {
    function foo(uint256 a) public returns (bytes4 value) {
        return msg.sig;
    }
}
// ====
// compileViaYul: also
// ----
// foo(uint256): 0x0 -> 0x2fbebd3800000000000000000000000000000000000000000000000000000000
