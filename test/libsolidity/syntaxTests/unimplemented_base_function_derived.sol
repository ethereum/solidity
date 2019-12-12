abstract contract a {
    function f() virtual public;
}
contract b is a {
    function f() public virtual override { a.f(); }
}
contract c is a,b {
    function f() public override(a, b) { a.f(); }
}
// ----
// TypeError: (118-121): Referencing unimplemented function "a.f".
// TypeError: (190-193): Referencing unimplemented function "a.f".
