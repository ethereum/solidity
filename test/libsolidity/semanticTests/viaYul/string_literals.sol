contract C {
    function short_dyn() public pure returns (string memory x) {
        x = "abc";
    }
    function long_dyn() public pure returns (string memory x) {
        x = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
    }
    function short_bytes_dyn() public pure returns (bytes memory x) {
        x = "abc";
    }
    function long_bytes_dyn() public pure returns (bytes memory x) {
        x = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
    }
    function bytesNN() public pure returns (bytes3 x) {
        x = "abc";
    }
    function bytesNN_padded() public pure returns (bytes4 x) {
        x = "abc";
    }
}
// ====
// compileViaYul: true
// ----
// short_dyn() -> 0x20, 3, "abc"
// long_dyn() -> 0x20, 80, "12345678901234567890123456789012", "34567890123456789012345678901234", "5678901234567890"
// short_bytes_dyn() -> 0x20, 3, "abc"
// long_bytes_dyn() -> 0x20, 80, "12345678901234567890123456789012", "34567890123456789012345678901234", "5678901234567890"
// bytesNN() -> 0x6162630000000000000000000000000000000000000000000000000000000000
// bytesNN_padded() -> 0x6162630000000000000000000000000000000000000000000000000000000000
