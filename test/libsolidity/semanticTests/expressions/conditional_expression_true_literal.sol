contract test {
    function f() public returns(uint d) {
        return true ? 5 : 10;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 5
