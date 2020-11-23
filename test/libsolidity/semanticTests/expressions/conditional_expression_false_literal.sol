contract test {
    function f() public returns(uint d) {
        return false ? 5 : 10;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 10
