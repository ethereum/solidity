pragma abicoder v2;

contract C {
    uint[] s;
    uint[2] n;

    function f_memory(uint[][] calldata a) public returns (uint[][] memory) {
        return a;
    }

    function f_encode(uint[][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function f_which(uint[][] calldata a, uint which) public returns (uint[] memory) {
        return a[which];
    }
}

// ----
// f_memory(uint256[][]): 0x20, 2, 0x40, 0x40, 2, 1, 2 -> 0x20, 2, 0x40, 0xa0, 2, 1, 2, 2, 1, 2
// f_memory(uint256[][]): 0x20, 2, 0x40, 0x60, 2, 1, 2 -> 0x20, 2, 0x40, 0xa0, 2, 1, 2, 1, 2
// f_memory(uint256[][]): 0x20, 2, 0, 0x60, 2, 1, 2 -> 0x20, 2, 0x40, 0x60, 0, 1, 2
// f_memory(uint256[][]): 0x20, 2, 0, 0x60, 2, 2, 2 -> FAILURE
// f_encode(uint256[][]): 0x20, 2, 0x40, 0x40, 2, 1, 2 -> 0x20, 0x0140, 0x20, 2, 0x40, 0xa0, 2, 1, 2, 2, 1, 2
// f_encode(uint256[][]): 0x20, 2, 0x40, 0x60, 2, 1, 2 -> 0x20, 0x0120, 0x20, 2, 0x40, 0xa0, 2, 1, 2, 1, 2
// f_encode(uint256[][]): 0x20, 2, 0, 0x60, 2, 1, 2 -> 0x20, 0xe0, 0x20, 2, 0x40, 0x60, 0, 1, 2
// f_encode(uint256[][]): 0x20, 2, 0, 0x60, 2, 2, 2 -> FAILURE
// f_which(uint256[][],uint256): 0x40, 0, 2, 0x40, 0x40, 2, 1, 2 -> 0x20, 2, 1, 2
// f_which(uint256[][],uint256): 0x40, 1, 2, 0x40, 0x40, 2, 1, 2 -> 0x20, 2, 1, 2
// f_which(uint256[][],uint256): 0x40, 0, 2, 0x40, 0x60, 2, 1, 2 -> 0x20, 2, 1, 2
// f_which(uint256[][],uint256): 0x40, 1, 2, 0x40, 0x60, 2, 1, 2 -> 0x20, 1, 2
// f_which(uint256[][],uint256): 0x40, 0, 2, 0, 0x60, 2, 1, 2 -> 0x20, 0
// f_which(uint256[][],uint256): 0x40, 1, 2, 0, 0x60, 2, 1, 2 -> 0x20, 1, 2
// f_which(uint256[][],uint256): 0x40, 1, 2, 0, 0x60, 2, 2, 2 -> FAILURE
