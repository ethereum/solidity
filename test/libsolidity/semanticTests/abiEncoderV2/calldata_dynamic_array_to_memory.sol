pragma abicoder v2;

contract C {
    function f(uint[][] calldata a) public returns (uint[][] memory) {
        return a;
    }

    function g(uint[][][] calldata a) public returns (uint[][][] memory) {
        return a;
    }

    function h(uint[2][][] calldata a) public returns (uint[2][][] memory) {
        return a;
    }
}

// ----
// f(uint256[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8 -> 0x20, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8
// f(uint256[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8, 9 -> 0x20, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8
// f(uint256[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8 -> FAILURE
// g(uint256[][][]): 0x20, 2, 0x40, 0x60, 0, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8 -> 0x20, 2, 0x40, 0x60, 0, 2, 0x40, 0xa0, 2, 5, 6, 2, 7, 8
// g(uint256[][][]): 0x20, 2, 0x40, 0x60, 0, 2, 0x40, 0xa0, 2, 5, 6, 2, 7 -> FAILURE
// h(uint256[2][][]): 0x20, 2, 0x40, 0x60, 0, 2, 5, 6, 7, 8 -> 0x20, 2, 0x40, 0x60, 0, 2, 5, 6, 7, 8
// h(uint256[2][][]): 0x20, 2, 0x40, 0x60, 0, 2, 5, 6, 7, 8, 9 -> 0x20, 2, 0x40, 0x60, 0, 2, 5, 6, 7, 8
// h(uint256[2][][]): 0x20, 2, 0x40, 0x60, 0, 2, 5, 6, 7 -> FAILURE
