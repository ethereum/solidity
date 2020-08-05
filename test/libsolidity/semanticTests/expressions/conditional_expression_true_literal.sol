contract test {
    function f() public returns(uint d) {
        return true ? 5 : 10;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 5
