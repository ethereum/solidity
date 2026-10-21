interface IX {
    function f() external returns (uint256);
}


contract A {
    function f() public virtual returns (uint256) { return 1; }
}


contract B is A {
    function f() public virtual override returns (uint256) { return super.f(); }
}


// IX.f() is not implemented, so it is not a candidate at all and `super.f()` resolves
// to A.f(). This is the common interface case and must compile.
contract D is A, IX, B {
    function f() public override(A, IX, B) returns (uint256) { return super.f(); }
}
// ----
