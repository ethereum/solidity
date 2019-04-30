pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[][] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(uint256[][] calldata s) external view returns (bytes memory) {
        return this.f(s);
    }
    function h(uint8[][] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function i(uint8[][] calldata s) external view returns (bytes memory) {
        return this.h(s);
    }
    function j(bytes[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function k(bytes[] calldata s) external view returns (bytes memory) {
        return this.j(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[][]): 0x20, 2, 0x40, 0xC0, 3, 13, 17, 23, 4, 27, 31, 37, 41 -> 32, 416, 32, 2, 64, 192, 3, 13, 17, 23, 4, 27, 31, 37, 41
// g(uint256[][]): 0x20, 2, 0x40, 0xC0, 3, 13, 17, 23, 4, 27, 31, 37, 41 -> 32, 416, 32, 2, 64, 192, 3, 13, 17, 23, 4, 27, 31, 37, 41
// h(uint8[][]): 0x20, 2, 0x40, 0xC0, 3, 13, 17, 23, 4, 27, 31, 37, 41 -> 32, 416, 32, 2, 64, 192, 3, 13, 17, 23, 4, 27, 31, 37, 41
// i(uint8[][]): 0x20, 2, 0x40, 0xC0, 3, 13, 17, 23, 4, 27, 31, 37, 41 -> 32, 416, 32, 2, 64, 192, 3, 13, 17, 23, 4, 27, 31, 37, 41
// j(bytes[]): 0x20, 2, 0x40, 0x63, 3, hex"131723", 4, hex"27313741" -> 32, 256, 32, 2, 64, 128, 3, left(0x131723), 4, left(0x27313741)
// k(bytes[]): 0x20, 2, 0x40, 0x63, 3, hex"131723", 4, hex"27313741" -> 32, 256, 32, 2, 64, 128, 3, left(0x131723), 4, left(0x27313741)
