contract Test {
    bytes x;
    function set(bytes memory _a) public { x = _a; }
}
// ----
// set(bytes): 0x20, 3, "abc"
// storageEmpty -> 0
// set(bytes): 0x20, 0
// storageEmpty -> 1
// set(bytes): 0x20, 31, "1234567890123456789012345678901"
// storageEmpty -> 0
// set(bytes): 0x20, 36, "12345678901234567890123456789012", "XXXX"
// storageEmpty -> 0
// set(bytes): 0x20, 3, "abc"
// storageEmpty -> 0
// set(bytes): 0x20, 0
// storageEmpty -> 1
// set(bytes): 0x20, 3, "abc"
// storageEmpty -> 0
// set(bytes): 0x20, 36, "12345678901234567890123456789012", "XXXX"
// storageEmpty -> 0
// set(bytes): 0x20, 0
// storageEmpty -> 1
// set(bytes): 0x20, 66, "12345678901234567890123456789012", "12345678901234567890123456789012", "12"
// gas irOptimized: 111895
// gas legacy: 112734
// gas legacyOptimized: 112110
// storageEmpty -> 0
// set(bytes): 0x20, 3, "abc"
// storageEmpty -> 0
// set(bytes): 0x20, 0
// storageEmpty -> 1
