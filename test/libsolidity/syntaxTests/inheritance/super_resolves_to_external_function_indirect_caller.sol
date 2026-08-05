contract A {
    function f() public virtual returns (uint256) { return 1; }
}


contract X {
    function f() external virtual returns (uint256) { return 100; }
}


// B takes no part in the override chain, it only sits above X in the
// linearization of D, which is [D, B, X, A].
contract B is A {
    function g() public returns (uint256) { return super.f(); }
}


contract D is A, X, B {
    function f() public override(A, X) returns (uint256) { return 7; }
}
// ----
// TypeError 8476: (351-358): In contract "D", this "super" call resolves to external function "X.f", which cannot be called internally. Make "X.f" public, or change the order of base contracts in "D".
