contract C {
    function f(bytes memory data) public pure returns(uint, bytes memory) {
        return abi.decode(data, (uint, bytes));
    }
}

// ----
// f(bytes): 0x20, 0x20 * 4, 33, 0x40, 7, "abcdefg" -> 33, 0x40, 7, "abcdefg"
// f(bytes):"32, 128, 33, 64, 7, abcdefg" -> "33, 64, 7, abcdefg"
