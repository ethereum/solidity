interface Parent {
    function test() external pure returns (uint256);
}

interface SubA is Parent {}
interface SubB is Parent {}

contract C is SubA, SubB {
    function test() external override pure returns (uint256) { return 42; }
}
// ----
