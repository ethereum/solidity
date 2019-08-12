contract Test {
    function test() public returns (uint ret) { return uint(address(Test(address(0x11223344556677889900112233445566778899001122)))); }
}
// ====
// compileViaYul: also
// ----
// test() -> 0x0000000000000000000000003344556677889900112233445566778899001122
