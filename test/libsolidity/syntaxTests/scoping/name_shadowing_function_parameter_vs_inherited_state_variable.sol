contract Base {
    uint256 value = 10;
}
contract Child is Base {
    function f(uint256 value) public pure returns (uint256) {
        return value;
    }
}
// ----
// Warning 2519: (82-95): This declaration shadows an existing declaration.
