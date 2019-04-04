pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(uint256[][2] calldata s, uint256 which) external view returns (bytes memory) {
        return this.f(s[which]);
    }
    function h(uint8[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function i(uint8[][2] calldata s, uint256 which) external view returns (bytes memory) {
        return this.h(s[which]);
    }
    function j(bytes calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function k(bytes[2] calldata s, uint256 which) external view returns (bytes memory) {
        return this.j(s[which]);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[]): 32, 3, 42, 23, 87 -> 32, 160, 32, 3, 42, 23, 87
// g(uint256[][2],uint256): 0x40, 0, 0x40, 0xC0, 3, 42, 23, 87, 4, 11, 13, 17 -> 32, 160, 32, 3, 42, 23, 87
// g(uint256[][2],uint256): 0x40, 1, 0x40, 0xC0, 3, 42, 23, 87, 4, 11, 13, 17, 27 -> 32, 192, 32, 4, 11, 13, 17, 27
// h(uint8[]): 32, 3, 42, 23, 87 -> 32, 160, 32, 3, 42, 23, 87
// i(uint8[][2],uint256): 0x40, 0, 0x40, 0xC0, 3, 42, 23, 87, 4, 11, 13, 17 -> 32, 160, 32, 3, 42, 23, 87
// i(uint8[][2],uint256): 0x40, 1, 0x40, 0xC0, 3, 42, 23, 87, 4, 11, 13, 17, 27 -> 32, 192, 32, 4, 11, 13, 17, 27
// j(bytes): 32, 3, hex"AB11FF" -> 32, 96, 32, 3, left(0xAB11FF)
// k(bytes[2],uint256): 0x40, 0, 0x40, 0x63, 3, hex"AB11FF", 4, hex"FF791432" -> 32, 96, 32, 3, left(0xAB11FF)
// k(bytes[2],uint256): 0x40, 1, 0x40, 0x63, 3, hex"AB11FF", 4, hex"FF791432" -> 32, 96, 32, 4, left(0xFF791432)
