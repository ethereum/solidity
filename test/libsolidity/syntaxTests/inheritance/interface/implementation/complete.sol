interface ParentA {
    function testA() external returns (uint256);
}

interface ParentB {
    function testB() external returns (uint256);
}

interface Sub is ParentA, ParentB {
    function testSub() external returns (uint256);
}

contract SubImpl is Sub {
    function testA() external override returns (uint256) { return 12; }
    function testB() external override(ParentB) returns (uint256) { return 42; }
    function testSub() external override returns (uint256) { return 99; }
}
