contract c {
    function f() public {}
}
contract d {
    function g() public { c(address(0)).f(); }
}
// ----
