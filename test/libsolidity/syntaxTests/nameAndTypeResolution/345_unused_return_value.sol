contract test {
    function g() public returns (uint) {}
    function f() public {
        g();
    }
}
// ----
