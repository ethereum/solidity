abstract contract a {
    function f() virtual public;
}
contract b is a {
    function f() public override { super.f(); }
}
// ----
// TypeError 9582: (110-117): Member "f" not found or not visible after argument-dependent lookup in type(contract super b).
