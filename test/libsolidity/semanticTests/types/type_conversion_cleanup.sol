contract Test {
    function test() public returns (uint ret) { return uint(uint160(address(uint160(uint128(type(uint200).max))))); }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() -> 0xffffffffffffffffffffffffffffffff
