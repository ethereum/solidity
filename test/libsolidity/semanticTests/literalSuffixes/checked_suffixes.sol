function checkedSuffix(uint8 x) pure suffix returns (uint8) {
    return x + 10;
}

contract C {
    function testCheckedSuffix() public pure returns (uint8) {
        return 250 checkedSuffix;
    }

    function testCheckedSuffixInUncheckedBlock() public pure returns (uint8) {
        unchecked {
            return 250 checkedSuffix;
        }
    }
}
// ----
// testCheckedSuffix() -> FAILURE, hex"4e487b71", 0x11
// testCheckedSuffixInUncheckedBlock() -> FAILURE, hex"4e487b71", 0x11
