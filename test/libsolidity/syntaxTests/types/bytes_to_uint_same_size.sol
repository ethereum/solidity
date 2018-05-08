contract C {
    function f() public pure returns (uint256) {
        return uint256(bytes32(uint256(0)));
    }
    function g() public pure returns (uint128) {
        return uint128(bytes16(uint128(0)));
    }
    function h() public pure returns (uint64) {
        return uint64(bytes8(uint64(0)));
    }
    function i() public pure returns (uint32) {
        return uint32(bytes4(uint32(0)));
    }
    function j() public pure returns (uint16) {
        return uint16(bytes2(uint16(0)));
    }
    function k() public pure returns (uint8) {
        return uint8(bytes1(uint8(0)));
    }
}
