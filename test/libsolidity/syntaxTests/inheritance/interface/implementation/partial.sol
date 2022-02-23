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
    function testSub() external override returns (uint256) { return 99; }
}

// ----
// TypeError 3656: (234-407): Contract "SubImpl" should be marked as abstract.
