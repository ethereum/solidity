contract Test {
    function test() public returns (uint ret) { return uint(uint160(address(uint160(type(uint200).max)))); }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() -> 0xffffffffffffffffffffffffffffffffffffffff
