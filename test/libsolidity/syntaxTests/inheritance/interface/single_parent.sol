interface Super {
    function test() external returns (uint256);
}

interface Sub is Super {}
