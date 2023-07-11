contract test {
    function f() public returns(int d) { return (-2) ** 3; }
}
// ----
// f() -> -8
