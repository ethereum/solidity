contract c {
    function f() public {}
}
contract d {
    function g() public { c(0).f(); }
}
// ----
