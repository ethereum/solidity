interface Parent {
    function test() external returns (uint256);
}

interface SubA is Parent {
    function test() external override returns (uint256);
}

interface SubB is Parent {
    function test() external override returns (uint256);
}

contract C is SubA, SubB {
    function test() external override(SubA, SubB) returns (uint256) { return 42; }
}
