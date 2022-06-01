contract C {
    string[4] public staticArray = ["foo", "", "a very long string that needs more than 32 bytes", "bar"];
    string[] public dynamicArray = ["foo", "", "a very long string that needs more than 32 bytes", "bar"];
}

// ====
// compileViaYul: also
// ----
// staticArray(uint256): 0 -> 0x20, 3, "foo"
// staticArray(uint256): 1 -> 0x20, 0
// staticArray(uint256): 2 -> 0x20, 0x30, 0x612076657279206c6f6e6720737472696e672074686174206e65656473206d6f, 0x7265207468616E20333220627974657300000000000000000000000000000000
// staticArray(uint256): 3 -> 0x20, 3, "bar"
// dynamicArray(uint256): 0 -> 0x20, 3, "foo"
// dynamicArray(uint256): 1 -> 0x20, 0
// dynamicArray(uint256): 2 -> 0x20, 0x30, 0x612076657279206c6f6e6720737472696e672074686174206e65656473206d6f, 0x7265207468616E20333220627974657300000000000000000000000000000000
// dynamicArray(uint256): 3 -> 0x20, 3, "bar"
