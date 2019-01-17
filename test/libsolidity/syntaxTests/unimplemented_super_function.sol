contract a {
    function f() public;
}
contract b is a {
    function f() public { super.f(); }
}
// ----
// TypeError: (84-91): Member "f" not found or not visible after argument-dependent lookup in contract super b.
