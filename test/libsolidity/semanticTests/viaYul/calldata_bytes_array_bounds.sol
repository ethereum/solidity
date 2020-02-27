pragma experimental ABIEncoderV2;
contract C {
    function f(bytes[] calldata a, uint256 i) external returns (uint) {
        return uint8(a[0][i]);
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes[],uint256): 0x40, 0, 1, 0x20, 2, hex"6162" -> 0x61
// f(bytes[],uint256): 0x40, 1, 1, 0x20, 2, hex"6162" -> 0x62
// f(bytes[],uint256): 0x40, 2, 1, 0x20, 2, hex"6162" -> FAILURE
