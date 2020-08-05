contract A {
    function f(uint256[] calldata a) external virtual returns (uint256) {
        return a[0];
    }
}


contract B is A {
    function f(uint256[] memory a) public override returns (uint256) {
        return a[1];
    }

    function g() public returns (uint256) {
        uint256[] memory m = new uint256[](2);
        m[0] = 42;
        m[1] = 23;
        return A(this).f(m);
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 23
