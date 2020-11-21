contract C {
    function f() public pure returns (uint) {
        uint x;
        return x;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0
