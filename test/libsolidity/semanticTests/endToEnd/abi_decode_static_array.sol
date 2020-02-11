contract C {
    function f(bytes calldata data) external pure returns(uint[2][3] memory) {
        return abi.decode(data, (uint[2][3]));
    }
}

// ----
// f(bytes): 0x20, 6 * 0x20, 1, 2, 3, 4, 5, 6 -> 1, 2, 3, 4, 5, 6
// f(bytes):"32, 192, 1, 2, 3, 4, 5, 6" -> "1, 2, 3, 4, 5, 6"
