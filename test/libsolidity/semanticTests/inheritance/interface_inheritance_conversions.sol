interface Parent {
    function parentFun() external returns (uint256);
}

interface SubA is Parent {
    function subAFun() external returns (uint256);
}

interface SubB is Parent {
    function subBFun() external returns (uint256);
}

contract C is SubA, SubB {
    function parentFun() override external returns (uint256) { return 1; }
    function subAFun() override external returns (uint256) { return 2; }
    function subBFun() override external returns (uint256) { return 3; }

    function convertParent() public returns (uint256) {
        return this.parentFun();
    }

    function convertSubA() public returns (uint256, uint256) {
        return (this.parentFun(), this.subAFun());
    }

    function convertSubB() public returns (uint256, uint256) {
        return (this.parentFun(), this.subBFun());
    }
}
// ----
// convertParent() -> 1
// gas irOptimized: 21787
// convertSubA() -> 1, 2
// gas irOptimized: 22450
// gas legacy: 23286
// convertSubB() -> 1, 3
// gas irOptimized: 22321
// gas legacy: 23154
