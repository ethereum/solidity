pragma experimental ABIEncoderV2;
contract C {
    function f(bytes[] calldata a, uint256 i) external returns(uint) {
        return uint8(a[0][i]);
    }
}

// ----
// f(bytes[],uint256): 0x40, 0, 1, 0x20, 2, "ab", 30, 0 -> 97
// f(bytes[],uint256): 0x40, 1, 1, 0x20, 2, "ab", 30, 0 -> 98
// f(bytes[],uint256): 0x40, 2, 1, 0x20, 2, "ab", 30, 0 -> FAILURE
