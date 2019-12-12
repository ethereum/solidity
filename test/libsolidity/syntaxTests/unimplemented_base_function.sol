abstract contract a {
    function f() virtual public;
}
contract b is a {
    function f() public override { a.f(); }
}
// ----
// TypeError: (110-113): Referencing unimplemented function "a.f".
