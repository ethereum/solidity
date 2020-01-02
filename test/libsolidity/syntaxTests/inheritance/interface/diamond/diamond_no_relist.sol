interface Parent {
    function test() external returns (uint256);
}

interface SubA is Parent {}
interface SubB is Parent {}

contract C is SubA, SubB {
    function test() external override returns (uint256) { return 42; }
}
