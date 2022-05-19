contract C {
    bytes s = "abcdefghabcdefgh";
    bytes sLong = "abcdefghabcdefghabcdefghabcdefgh";

    function fromMemory(bytes memory m) public returns (bytes16) {
        return bytes16(m);
    }
    function fromCalldata(bytes calldata c) external returns (bytes16) {
        return bytes16(c);
    }
    function fromStorage() external returns (bytes16) {
        return bytes16(s);
    }
    function fromStorageLong() external returns (bytes32) {
        return bytes32(sLong);
    }
    function fromSlice(bytes calldata c) external returns (bytes8) {
        return bytes8(c[1:9]);
    }
}
// ----
// fromMemory(bytes): 0x20, 16, "abcdefghabcdefgh" -> "abcdefghabcdefgh"
// fromCalldata(bytes): 0x20, 16, "abcdefghabcdefgh" -> "abcdefghabcdefgh"
// fromStorage() -> "abcdefghabcdefgh"
// fromStorageLong() -> "abcdefghabcdefghabcdefghabcdefgh"
// fromSlice(bytes): 0x20, 16, "abcdefghabcdefgh" -> "bcdefgha"
