pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[3] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(uint256[3] calldata s) external view returns (bytes memory) {
        return this.f(s);
    }
    function h(uint8[3] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function i(uint8[3] calldata s) external view returns (bytes memory) {
        return this.h(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// g(uint256[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// h(uint8[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// i(uint8[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// h(uint8[3]): 0xFF23, 0x1242, 0xAB87 -> FAILURE
// i(uint8[3]): 0xAB23, 0x1242, 0xFF87 -> FAILURE
