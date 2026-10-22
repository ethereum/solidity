abstract contract Base {
    uint256 value = 10;
}
contract Child is Base {
    function f() public pure returns (uint256) {
        uint256 value = 99;
        return value;
    }
}
// ---- 
// Warning 2519: (133-146): This declaration shadows an existing declaration.
