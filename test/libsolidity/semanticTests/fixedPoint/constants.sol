contract C {
    function f() public pure returns (fixed) {
        return 99.101 * 3.1;
    }
    function g() public pure returns (fixed128x2) {
        return fixed128x2(-1/3);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 307.213100000000000000
// g() -> -0.33
