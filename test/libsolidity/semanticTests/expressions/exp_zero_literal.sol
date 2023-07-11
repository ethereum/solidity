contract test {
    function f() public returns(uint d) { return 0 ** 0; }
}
// ----
// f() -> 1
