interface ParentA {
    function testA() external pure returns (uint256);
}

interface ParentB {
    function testB() external pure returns (uint256);
}

interface Sub is ParentA, ParentB {
    function testSub() external pure returns (uint256);
}

contract SubImpl is Sub {
    function testA() external pure override returns (uint256) { return 12; }
    function testB() external pure override(ParentB) returns (uint256) { return 42; }
    function testSub() external pure override returns (uint256) { return 99; }
}
