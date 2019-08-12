contract test {
    function f() public returns(uint d) { return 2 ** 3; }
}
// ====
// compileViaYul: also
// ----
// f() -> 8
