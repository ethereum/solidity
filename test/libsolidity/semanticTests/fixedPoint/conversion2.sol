contract C {
    function test() public pure returns (fixed64x3, fixed128x4, ufixed64x4) {
        fixed64x4 x = fixed64x4(-1.12346789);
        return (fixed64x3(x), fixed128x4(x), ufixed64x4(x));
    }
}
// ====
// compileViaYul: also
// ----
// test() -> -1.123, -1.1234, 1844674407370954.0382
