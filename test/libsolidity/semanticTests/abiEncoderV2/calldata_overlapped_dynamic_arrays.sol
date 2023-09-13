pragma abicoder v2;

contract C {
    uint[] s;
    uint[2] n;

    function f_memory(uint[] calldata a, uint[2] calldata b) public returns (uint[] memory, uint[2] memory) {
        return (a, b);
    }

    function f_encode(uint[] calldata a, uint[2] calldata b) public returns (bytes memory) {
        return abi.encode(a, b);
    }

    function f_which(uint[] calldata a, uint[2] calldata b, uint which) public returns (bytes memory) {
        return abi.encode(a[which], b[1]);
    }

    function f_storage(uint[] calldata a, uint[2] calldata b ) public returns (bytes memory) {
        s = a;
        n = b;
        return abi.encode(s);
    }
}
// ----
// f_memory(uint256[],uint256[2]): 0x20, 1, 2 -> 0x60, 0x01, 0x02, 1, 2
// f_memory(uint256[],uint256[2]): 0x40, 1, 2, 5, 6 -> 0x60, 1, 2, 2, 5, 6
// f_memory(uint256[],uint256[2]): 0x40, 1, 2, 5 -> FAILURE
// f_encode(uint256[],uint256[2]): 0x20, 1, 2 -> 0x20, 0xa0, 0x60, 1, 2, 1, 2
// f_encode(uint256[],uint256[2]): 0x40, 1, 2, 5, 6 -> 0x20, 0xc0, 0x60, 1, 2, 2, 5, 6
// f_encode(uint256[],uint256[2]): 0x40, 1, 2, 5 -> FAILURE
// f_which(uint256[],uint256[2],uint256): 0x40, 1, 2, 1, 5 -> 0x20, 0x40, 5, 2
// f_which(uint256[],uint256[2],uint256): 0x40, 1, 2, 1, 5, 6 -> 0x20, 0x40, 5, 2
// f_which(uint256[],uint256[2],uint256): 0x40, 1, 2, 1 -> FAILURE
// f_storage(uint256[],uint256[2]): 0x20, 1, 2 -> 0x20, 0x60, 0x20, 1, 2
// gas irOptimized: 111642
// gas legacy: 112944
// gas legacyOptimized: 112092
// f_storage(uint256[],uint256[2]): 0x40, 1, 2, 5, 6 -> 0x20, 0x80, 0x20, 2, 5, 6
// f_storage(uint256[],uint256[2]): 0x40, 1, 2, 5 -> FAILURE
