contract A {
    function f() public virtual {}
}
abstract contract B {
    function f() public virtual;
}
contract C is A, B {
    function f() public override(A, B) {
        super.f(); // super should skip the unimplemented B.f() and call A.f() instead.
    }
}
// ----
