abstract contract a {
    function f() public;
}
contract b is a {
    function f() public override { super.f(); }
}
// ----
// TypeError: (102-109): Member "f" not found or not visible after argument-dependent lookup in contract super b.
