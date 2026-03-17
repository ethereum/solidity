library L {
    function fi(uint256 a, uint256 b, uint256 c) internal returns (uint256) {
        return 1 * a + 2 * b + 3 * c;
    }
    function fe(uint256 a, uint256 b, uint256 c) external returns (uint256) {
        return 1 * a + 2 * b + 3 * c;
    }
}

contract C {
    function internalOrdered() public returns (uint256) {
        return L.fi({a: 1, b: 2, c:3});
    }
    function internalUnordered() public returns (uint256) {
        return L.fi({c: 3, a: 1, b: 2});
    }
    function externalOrdered() external returns (uint256) {
        return L.fe({a: 1, b: 2, c:3});
    }
    function externalUnordered() public returns (uint256) {
        return L.fe({c: 3, a: 1, b: 2});
    }
}
// ----
// library: L
// internalOrdered() -> 14
// internalUnordered() -> 14
// externalOrdered() -> 14
// externalUnordered() -> 14
