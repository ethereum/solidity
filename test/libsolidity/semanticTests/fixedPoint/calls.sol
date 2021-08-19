contract C {
    function f(fixed128x2 x) public view returns (ufixed64x4) {
        return ufixed64x4(ufixed128x4(ufixed128x2(x)));
    }
    function test() public view returns (ufixed64x4 x, fixed y) {
        fixed64x1 a = 1234.3;
        x = this.f(a);
        y = this.f(-1234.3);
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 1234.3000, 1844674407369720.861600000000000000
