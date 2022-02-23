contract A {
    function f() public virtual {}
}
abstract contract B {
    function f() public virtual;
}
contract C is A, B {
    function f() public override(A, B) {
        // This is fine. The unimplemented B.f() is not used.
    }
}
// ----
