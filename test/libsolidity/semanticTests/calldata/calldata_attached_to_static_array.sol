pragma abicoder v2;

library L {
    function reverse(uint[2] calldata _a) internal pure returns (uint, uint) {
        return (_a[1], _a[0]);
    }
}

contract C {
    using L for uint[2];

    function test(uint, uint[2] calldata _a, uint) external pure returns (uint, uint) {
        return _a.reverse();
    }
}
// ----
// test(uint256,uint256[2],uint256): 7, 66, 77, 4 -> 77, 66
