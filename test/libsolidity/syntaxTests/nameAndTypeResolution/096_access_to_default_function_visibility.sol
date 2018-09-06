contract c {
    function f() public {}
}
contract d {
    function g() public { c(0).f(); }
}
// ----
// Warning: (17-39): Function state mutability can be restricted to pure
