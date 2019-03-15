pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(uint256[] calldata s) external view returns (bytes memory) {
        return this.f(s);
    }
    function h(uint8[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function i(uint8[] calldata s) external view returns (bytes memory) {
        return this.h(s);
    }
    function j(bytes calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function k(bytes calldata s) external view returns (bytes memory) {
        return this.j(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[]): 32, 3, 23, 42, 87 -> 32, 160, 32, 3, 23, 42, 87
// g(uint256[]): 32, 3, 23, 42, 87 -> 32, 160, 32, 3, 23, 42, 87
// h(uint8[]): 32, 3, 23, 42, 87 -> 32, 160, 32, 3, 23, 42, 87
// i(uint8[]): 32, 3, 23, 42, 87 -> 32, 160, 32, 3, 23, 42, 87
// h(uint8[]): 32, 3, 0xFF23, 0x1242, 0xAB87 -> FAILURE
// i(uint8[]): 32, 3, 0xAB23, 0x1242, 0xFF87 -> FAILURE
// j(bytes): 32, 3, hex"123456" -> 32, 96, 32, 3, left(0x123456)
// k(bytes): 32, 3, hex"AB33FF" -> 32, 96, 32, 3, left(0xAB33FF)
