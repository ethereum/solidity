contract Test {
    function test() public returns (uint ret) { return uint(address(uint128(0x11223344556677889900112233445566778899001122))); }
}
// ====
// compileViaYul: also
// ----
// test() -> 158887387085137674884660775897412931874
