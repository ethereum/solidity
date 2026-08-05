contract A {
    function f() public virtual returns (uint256) { return 1; }
}


contract X {
    function f() external virtual returns (uint256) { return 100; }
}


// Binding super.f to an internal function pointer rather than calling it directly.
contract B is A {
    function f() public virtual override returns (uint256) {
        function () internal returns (uint256) p = super.f;
        return p();
    }
}


contract D is A, X, B {
    function f() public override(A, X, B) returns (uint256) { return super.f(); }
}
// ----
// TypeError 8476: (380-387): In contract "D", this "super" call resolves to external function "X.f", which cannot be called internally. Make "X.f" public, or change the order of base contracts in "D".
