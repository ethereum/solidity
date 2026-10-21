contract A {
    function f() public virtual returns (uint256) { return 1; }
}


contract X {
    function f() external virtual returns (uint256) { return 100; }
}


contract B is A {
    function f() public virtual override returns (uint256) { return super.f(); }
}


// No code is generated for an abstract contract, so the error only appears once a
// concrete contract fixes the linearization.
abstract contract D is A, X, B {
    function f() public virtual override(A, X, B) returns (uint256) { return super.f(); }
}
// ----
