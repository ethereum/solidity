contract C {
    bytes s = "abcdefghabcdefghabcdefghabcdefg";

    function fromMemory(bytes memory m) public returns (bytes16) {
        assembly { mstore(m, 14) }
        return bytes16(m);
    }
    function fromCalldata(bytes calldata c) external returns (bytes16) {
        return bytes16(c);
    }
    function fromStorage() external returns (bytes32) {
        return bytes32(s);
    }
    function fromSlice(bytes calldata c) external returns (bytes8) {
        return bytes8(c[0:6]);
    }
}
// ====
// compileViaYul: true
// ----
// fromMemory(bytes): 0x20, 16, "abcdefghabcdefgh" -> "abcdefghabcdef\0\0"
// fromCalldata(bytes): 0x20, 15, "abcdefghabcdefgh" -> "abcdefghabcdefg\0"
// fromStorage() -> "abcdefghabcdefghabcdefghabcdefg\0"
// fromSlice(bytes): 0x20, 15, "abcdefghabcdefgh" -> "abcdef\0\0"
