contract C {
    bytes data;

    function f(bytes memory _data) public returns (uint256, bytes memory) {
        data = _data;
        return abi.decode(data, (uint256, bytes));
    }
}

// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0x80, 0x21, 0x40, 0x7, "abcdefg" -> 0x21, 0x40, 0x7, "abcdefg"
// gas irOptimized: 135918
// gas legacy: 137190
// gas legacyOptimized: 136082
