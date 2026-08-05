contract A {
    function f() public virtual returns (uint256) { return 1; }
}


contract B is A {
    function f() public virtual override returns (uint256) { return super.f(); }
}


contract X {
    function f() external virtual returns (uint256) { return 100; }
}


// C3 linearization of D is [D, B, X, A], so `super.f()` in B resolves to X.f(),
// which is external and cannot be called internally.
contract D is A, X, B {
    function f() public override(A, X, B) returns (uint256) { return super.f(); }
}
// ----
// TypeError 8476: (167-174): In contract "D", this "super" call resolves to external function "X.f", which cannot be called internally. Make "X.f" public, or change the order of base contracts in "D".
