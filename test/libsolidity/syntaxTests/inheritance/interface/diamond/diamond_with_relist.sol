interface Parent {
    function test() external pure returns (uint256);
}

interface SubA is Parent {
    function test() external pure override returns (uint256);
}

interface SubB is Parent {
    function test() external pure override returns (uint256);
}

contract C is SubA, SubB {
    function test() external pure override(SubA, SubB) returns (uint256) { return 42; }
}
// ----
