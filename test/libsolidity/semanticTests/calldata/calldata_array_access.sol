pragma abicoder               v2;
contract C {
    function f(uint256[] calldata x, uint256 i) external returns (uint256) {
        return x[i];
    }
    function f(uint256[][] calldata x, uint256 i, uint256 j) external returns (uint256) {
        return x[i][j];
    }
}
// ----
// f(uint256[],uint256): 0x40, 0, 0 -> FAILURE, hex"4e487b71", 0x32
// f(uint256[],uint256): 0x40, 0, 1, 23 -> 23
// f(uint256[],uint256): 0x40, 1, 1, 23 -> FAILURE, hex"4e487b71", 0x32
// f(uint256[],uint256): 0x40, 0, 2, 23, 42 -> 23
// f(uint256[],uint256): 0x40, 1, 2, 23, 42 -> 42
// f(uint256[],uint256): 0x40, 2, 2, 23, 42 -> FAILURE, hex"4e487b71", 0x32
// f(uint256[][],uint256,uint256): 0x60, 0, 0 -> FAILURE
// f(uint256[][],uint256,uint256): 0x60, 0, 0, 1, 0x20, 1, 23 -> 23
