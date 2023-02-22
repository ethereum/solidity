contract C {
    function f() public returns (bool) {
        return gasleft() > 0;
    }
}
// ----
// f() -> true
// f() -> true
// f() -> true
