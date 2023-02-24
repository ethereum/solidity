function uncheckedSuffix(uint8 x) pure suffix returns (uint8) {
    unchecked {
        return x + 10;
    }
}

contract C {
    function testUncheckedSuffix() public pure returns (uint8) {
        return 250 uncheckedSuffix;
    }

    function testUncheckedSuffixInUncheckedBlock() public pure returns (uint8) {
        unchecked {
            return 250 uncheckedSuffix;
        }
    }
}
// ----
// testUncheckedSuffix() -> 4
// testUncheckedSuffixInUncheckedBlock() -> 4
