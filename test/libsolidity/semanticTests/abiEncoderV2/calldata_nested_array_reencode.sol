pragma abicoder v2;

contract C {
    function f(uint[][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }
    function g(uint8[][][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }
    function h(uint16[][2][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }
    function i(uint16[][][1] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }
    function j(uint16[2][][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }
}
// ====
// revertStrings: debug
// ----
// f(uint256[][]): 0x20, 1, 0x20, 0 -> 0x20, 0x80, 0x20, 1, 0x20, 0
// f(uint256[][]): 0x20, 1, 0x20, 1 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// f(uint256[][]): 0x20, 1, 0x20, 2 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// f(uint256[][]): 0x20, 1, 0x20, 3 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// g(uint8[][][]): 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 0 -> 0x20, 0x01a0, 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 0
// g(uint8[][][]): 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access offset"
// g(uint8[][][]): 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 1, 0x20, 0 -> 0x20, 0x01e0, 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 1, 0x20, 0
// g(uint8[][][]): 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 1, 0x20, 0, 1 -> 0x20, 0x01e0, 0x20, 2, 0x40, 0x0140, 2, 0x40, 0x80, 1, 10, 2, 11, 12, 1, 0x20, 0
// h(uint16[][2][]): 0x20, 2, 0x40, 0x0120, 0x40, 0x80, 1, 10, 2, 11, 12, 0x40, 0x60, 0, 1, 13 -> 0x20, 0x0200, 0x20, 2, 0x40, 288, 0x40, 0x80, 1, 10, 2, 11, 12, 0x40, 0x60, 0, 1, 13
// h(uint16[][2][]): 0x20, 2, 0x40, 0x0120, 0x40, 0x80, 1, 10, 2, 11, 12, 0x40, 0x60, 0, 1 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// i(uint16[][][1]): 0x20, 0x20, 2, 0x40, 0x80, 1, 10, 2, 11, 12 -> 0x20, 0x0140, 0x20, 0x20, 2, 0x40, 0x80, 1, 10, 2, 11, 12
// i(uint16[][][1]): 0x20, 0x20, 2, 0x40, 0x80, 1, 10, 2, 11 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
// j(uint16[2][][]): 0x20, 2, 0x40, 0xa0, 1, 0x0a, 11, 2, 12, 13, 14, 15 -> 0x20, 0x0180, 0x20, 2, 0x40, 0xa0, 1, 10, 11, 2, 12, 13, 14, 15
// j(uint16[2][][]): 0x20, 2, 0x40, 0xa0, 1, 0x0a, 11, 2, 12, 13, 14 -> FAILURE, hex"08c379a0", 0x20, 0x1e, "Invalid calldata access stride"
