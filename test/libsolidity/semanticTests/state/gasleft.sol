contract C {
    function f() public returns (bool) {
        return gasleft() > 0;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> true
// f() -> true
// f() -> true
