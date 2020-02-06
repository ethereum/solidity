contract C {
    function f() public pure returns (uint) {
        uint x;
        return x;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
