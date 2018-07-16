contract c {
    function f() internal {}
}
contract d {
    function g() public { c(0).f(); }
}
// ----
// TypeError: (83-89): Member "f" not found or not visible after argument-dependent lookup in contract c.
