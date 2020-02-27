pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[] calldata x) external returns (uint256) {
        return x.length;
    }
    function f(uint256[][] calldata x) external returns (uint256 l1, uint256 l2, uint256 l3) {
        l1 = x.length;
        if (l1 > 0) l2 = x[0].length;
        if (l1 > 1) l3 = x[1].length;
    }
    function f(uint256[2] calldata x) external returns (uint256) {
        return x.length;
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256[]): 0x20, 0 -> 0
// f(uint256[]): 0x20, 1, 23 -> 1
// f(uint256[]): 0x20, 2, 23, 42 -> 2
// f(uint256[]): 0x20, 3, 23, 42, 17 -> 3
// f(uint256[2]): 23, 42 -> 2
// f(uint256[][]): 0x20, 0 -> 0, 0, 0
// f(uint256[][]): 0x20, 1, 0x20, 0 -> 1, 0, 0
// f(uint256[][]): 0x20, 1, 0x00 -> 1, 0, 0
// f(uint256[][]): 0x20, 1, 0x20, 1, 23 -> 1, 1, 0
// f(uint256[][]): 0x20, 1, 0x20, 2, 23, 42 -> 1, 2, 0
// f(uint256[][]): 0x20, 1, 0x40, 0, 2, 23, 42 -> 1, 2, 0
// f(uint256[][]): 0x20, 1, -32 -> 1, 1, 0
// f(uint256[][]): 0x20, 2, 0x40, 0x40, 2, 23, 42 -> 2, 2, 2
// f(uint256[][]): 0x20, 2, 0x40, 0xa0, 2, 23, 42, 0 -> 2, 2, 0
// f(uint256[][]): 0x20, 2, 0xA0, 0x40, 2, 23, 42, 0 -> 2, 0, 2
// f(uint256[][]): 0x20, 2, 0x40, 0xA0, 2, 23, 42, 1, 17 -> 2, 2, 1
// f(uint256[][]): 0x20, 2, 0x40, 0xA0, 2, 23, 42, 2, 17, 13 -> 2, 2, 2