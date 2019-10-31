contract a {
    function f() public;
}
contract b is a {
    function f() public override { super.f(); }
}
contract c is a,b {
    // No error here.
    function f() public override(a, b) { super.f(); }
}
// ----
// TypeError: (93-100): Member "f" not found or not visible after argument-dependent lookup in contract super b.
