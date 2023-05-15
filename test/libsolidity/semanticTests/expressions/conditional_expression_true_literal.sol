contract test {
    function f() public returns(uint d) {
        return true ? 5 : 10;
    }
}
// ----
// f() -> 5
