contract Base {
    uint256 private value = 10;
}
contract Child is Base {
    function f(uint256 value) public pure returns (uint256) {
        return value;
    }
}
// ----