contract C {
    function f() public returns (bool) {
        return gasleft() > 0;
    }
}
// ====
// compileToEOF: false
// ----
// f() -> true
// f() -> true
// f() -> true
