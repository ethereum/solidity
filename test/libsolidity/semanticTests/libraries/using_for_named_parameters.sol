library L {
    function multiply(uint256 self, uint256 x, uint256 y) internal pure returns (uint256, uint256) {
        return (self * x, self * y);
    }
}

contract C {
    using L for uint256;

    function ordered() external pure returns (uint256, uint256) {
        uint256 value = 1;
        return value.multiply({x: 2, y: 7});
    }
    function unordered() external pure returns (uint256, uint256) {
        uint256 value = 1;
        return value.multiply({y: 7, x: 2});
    }
}
// ----
// ordered() -> 2, 7
// unordered() -> 2, 7
