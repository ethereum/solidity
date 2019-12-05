contract A {
    function f() public virtual {}
}
contract B {
    function f() public virtual {}
}
contract C is A, B {
    function f() public override(A, B) {}
}

// ----
