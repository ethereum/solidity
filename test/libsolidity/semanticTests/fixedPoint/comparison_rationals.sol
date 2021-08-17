contract C {
    function test() public pure returns (bool) {
        require(1/2 == 1/2);
        require(fixed128x2(1/2 + 0.0000001) == 1/2);
        require(fixed128x2(1/3) < 1/2);
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// test() -> true