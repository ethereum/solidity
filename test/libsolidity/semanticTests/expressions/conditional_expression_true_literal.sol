contract test {
    function f() public returns(uint d) {
        return true ? 5 : 10;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 5
