contract A {
    function f(uint256 a, uint256 b, uint256 c) internal virtual returns (uint256) {
        return 1 * a + 2 * b + 3 * c;
    }
}

contract B is A {
    function f(uint256 b, uint256 a, uint256 c) internal override returns (uint256) {
        return 2 * b + 3 * a + 4 * c;
    }
    function baseOrdered() public returns (uint256) {
        return A.f({a: 1, b: 2, c: 3});
    }
    function baseUnordered() public returns (uint256) {
        return A.f({c: 3, a: 1, b: 2});
    }
    function overrideOrdered() public returns (uint256) {
        return f({b: 1, a: 2, c: 3});
    }
    function overrideUnordered() public returns (uint256) {
        return f({c: 3, b: 1, a: 2});
    }
}
// ----
// baseOrdered() -> 14
// baseUnordered() -> 14
// overrideOrdered() -> 20
// overrideUnordered() -> 20
