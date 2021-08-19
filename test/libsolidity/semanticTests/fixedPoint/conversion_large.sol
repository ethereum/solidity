contract C {
    function test() public pure returns (uint a) {
        ufixed256x77 x = 1.0000001234;
        return uint(x);
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 0
