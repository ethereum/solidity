pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[3] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(uint256[3][2] calldata s, uint256 which) external view returns (bytes memory) {
        return this.f(s[which]);
    }
    function h(uint8[3] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function i(uint8[3][2] calldata s, uint256 which) external view returns (bytes memory) {
        return this.h(s[which]);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// g(uint256[3][2],uint256): 23, 42, 87, 123, 142, 187, 0 -> 32, 96, 23, 42, 87
// g(uint256[3][2],uint256): 23, 42, 87, 123, 142, 187, 1 -> 32, 96, 123, 142, 187
// h(uint8[3]): 23, 42, 87 -> 32, 96, 23, 42, 87
// i(uint8[3][2],uint256): 23, 42, 87, 123, 142, 187, 0 -> 32, 96, 23, 42, 87
// i(uint8[3][2],uint256): 23, 42, 87, 123, 142, 187, 1 -> 32, 96, 123, 142, 187
