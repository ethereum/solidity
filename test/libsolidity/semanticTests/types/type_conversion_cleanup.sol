contract Test {
    function test() public returns (uint ret) { return uint(uint160(address(uint160(uint128(type(uint200).max))))); }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 0xffffffffffffffffffffffffffffffff
