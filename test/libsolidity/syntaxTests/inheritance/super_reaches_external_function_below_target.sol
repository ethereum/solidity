contract X {
    function f() external virtual returns (uint256) { return 100; }
}


contract A is X {
    function f() public virtual override returns (uint256) { return 1; }
}


// X.f() is external, but it sits below the function `super.f()` resolves to,
// so it is never the target.
contract B is A {
    function f() public override returns (uint256) { return super.f(); }
}
// ----
