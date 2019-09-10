pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[] calldata s1, uint256[] calldata s2, bool which) external pure returns (bytes memory) {
        if (which)
            return abi.encode(s1);
        else
            return abi.encode(s2);
    }
    function g(uint256[] calldata s1, uint256[] calldata s2, bool which) external view returns (bytes memory) {
        return this.f(s1, s2, which);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[],uint256[],bool): 0x60, 0xE0, true, 3, 23, 42, 87, 2, 51, 72 -> 32, 160, 0x20, 3, 23, 42, 87
// f(uint256[],uint256[],bool): 0x60, 0xE0, false, 3, 23, 42, 87, 2, 51, 72 -> 32, 128, 0x20, 2, 51, 72
// g(uint256[],uint256[],bool): 0x60, 0xE0, true, 3, 23, 42, 87, 2, 51, 72 -> 32, 160, 0x20, 3, 23, 42, 87
// g(uint256[],uint256[],bool): 0x60, 0xE0, false, 3, 23, 42, 87, 2, 51, 72 -> 32, 128, 0x20, 2, 51, 72
