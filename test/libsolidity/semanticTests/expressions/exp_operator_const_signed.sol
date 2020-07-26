contract test {
    function f() public returns(int d) { return (-2) ** 3; }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> -8
