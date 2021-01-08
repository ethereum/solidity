contract Test {
    bytes x;
    function set(bytes memory _a) public { x = _a; }
}
// ====
// compileViaYul: also
// ----
// set(bytes): 0x20, 3, "abc"
// storage_empty -> false
// set(bytes): 0x20, 0
// storage_empty -> true
// set(bytes): 0x20, 31, "1234567890123456789012345678901"
// storage_empty -> false
// set(bytes): 0x20, 36, "12345678901234567890123456789012", "XXXX"
// storage_empty -> false
// set(bytes): 0x20, 3, "abc"
// storage_empty -> false
// set(bytes): 0x20, 0
// storage_empty -> true
// set(bytes): 0x20, 3, "abc"
// storage_empty -> false
// set(bytes): 0x20, 36, "12345678901234567890123456789012", "XXXX"
// storage_empty -> false
// set(bytes): 0x20, 0
// storage_empty -> true
// set(bytes): 0x20, 66, "12345678901234567890123456789012", "12345678901234567890123456789012", "12"
// storage_empty -> false
// set(bytes): 0x20, 3, "abc"
// storage_empty -> false
// set(bytes): 0x20, 0
// storage_empty -> true
