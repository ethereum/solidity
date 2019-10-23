contract a {
    function f() public;
}
contract b is a {
    function f() public override { super.f(); }
}
// ----
// TypeError: (0-39): Contract "a" should be marked as abstract.
// TypeError: (93-100): Member "f" not found or not visible after argument-dependent lookup in contract super b.
