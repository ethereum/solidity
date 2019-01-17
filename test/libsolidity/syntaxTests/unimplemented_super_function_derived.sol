contract a {
    function f() public;
}
contract b is a {
    function f() public { super.f(); }
}
contract c is a,b {
    // No error here.
    function f() public { super.f(); }
}
// ----
// TypeError: (84-91): Member "f" not found or not visible after argument-dependent lookup in contract super b.
