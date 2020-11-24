contract C {
    function f() public pure returns (uint x) {
        return 7;
        x = 3;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 7
