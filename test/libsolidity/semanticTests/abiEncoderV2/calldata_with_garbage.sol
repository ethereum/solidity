pragma abicoder v2;

contract C {
    uint[] aTmp;
    uint[2] bTmp;

    function f_memory(uint[] calldata a) public returns (uint[] memory) {
        return a;
    }

    function f_encode(uint[] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function f_storage(uint[] calldata a) public returns (bytes memory) {
        aTmp = a;
        return abi.encode(aTmp);
    }

    function f_index(uint[] calldata a, uint which) public returns (uint) {
        return a[which];
    }

    function g_memory(uint[] calldata a, uint[2] calldata b) public returns (uint[] memory, uint[2] memory) {
        return (a, b);
    }

    function g_encode(uint[] calldata a, uint[2] calldata b) public returns (bytes memory) {
        return abi.encode(a, b);
    }

    function g_storage(uint[] calldata a, uint[2] calldata b) public returns (bytes memory) {
        aTmp = a;
        bTmp = b;
        return abi.encode(aTmp, bTmp);
    }

    function g_index(uint[] calldata a, uint[2] calldata b, uint which) public returns (uint, uint) {
        return (a[which], b[0]);
    }
}

// ----
// f_memory(uint256[]): 0x80, 9, 9, 9, 0 -> 0x20, 0
// f_memory(uint256[]): 0x80, 9, 9, 9, 1, 7 -> 0x20, 1, 7
// f_memory(uint256[]): 0x80, 9, 9, 9, 2, 7 -> FAILURE
// f_encode(uint256[]): 0x80, 9, 9, 9, 0 -> 0x20, 0x40, 0x20, 0
// f_encode(uint256[]): 0x80, 9, 9, 9, 1, 7 -> 0x20, 0x60, 0x20, 1, 7
// f_encode(uint256[]): 0x80, 9, 9, 9, 2, 7 -> FAILURE
// f_storage(uint256[]): 0x80, 9, 9, 9, 0 -> 0x20, 0x40, 0x20, 0
// f_storage(uint256[]): 0x80, 9, 9, 9, 1, 7 -> 0x20, 0x60, 0x20, 1, 7
// f_storage(uint256[]): 0x80, 9, 9, 9, 2, 7 -> FAILURE
// f_index(uint256[],uint256): 0xa0, 0, 9, 9, 9, 2, 7, 8 -> 7
// f_index(uint256[],uint256): 0xa0, 1, 9, 9, 9, 2, 7, 8 -> 8
// f_index(uint256[],uint256): 0xa0, 2, 9, 9, 9, 2, 7, 8 -> FAILURE, hex"4e487b71", 0x32
// g_memory(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 0 -> 0x60, 1, 2, 0
// g_memory(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 1, 7 -> 0x60, 1, 2, 1, 7
// g_memory(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 2, 7 -> FAILURE
// g_encode(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 0 -> 0x20, 0x80, 0x60, 1, 2, 0
// g_encode(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 1, 7 -> 0x20, 0xa0, 0x60, 1, 2, 1, 7
// g_encode(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 2, 7 -> FAILURE
// g_storage(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 0 -> 0x20, 0x80, 0x60, 1, 2, 0
// g_storage(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 1, 7 -> 0x20, 0xa0, 0x60, 1, 2, 1, 7
// g_storage(uint256[],uint256[2]): 0xc0, 1, 2, 9, 9, 9, 2, 7 -> FAILURE
// g_index(uint256[],uint256[2],uint256): 0xe0, 1, 2, 0, 9, 9, 9, 2, 7, 8 -> 7, 1
// g_index(uint256[],uint256[2],uint256): 0xe0, 1, 2, 1, 9, 9, 9, 2, 7, 8 -> 8, 1
// g_index(uint256[],uint256[2],uint256): 0xe0, 1, 2, 1, 9, 9, 9, 2, 7 -> FAILURE
