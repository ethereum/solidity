pragma experimental ABIEncoderV2;

contract C {
    function f(uint256[3] calldata s1, uint256[2] calldata s2, bool which) external pure returns (bytes memory) {
        if (which)
            return abi.encode(s1);
        else
            return abi.encode(s2);
    }
    function g(uint256[3] calldata s1, uint256[2] calldata s2, bool which) external view returns (bytes memory) {
        return this.f(s1, s2, which);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[3],uint256[2],bool): 23, 42, 87, 51, 72, true -> 32, 96, 23, 42, 87
// f(uint256[3],uint256[2],bool): 23, 42, 87, 51, 72, false -> 32, 64, 51, 72
// g(uint256[3],uint256[2],bool): 23, 42, 87, 51, 72, true -> 32, 96, 23, 42, 87
// g(uint256[3],uint256[2],bool): 23, 42, 87, 51, 72, false -> 32, 64, 51, 72
