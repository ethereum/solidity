contract C {
    bytes s = "abcdefghabcdefghabcdefghabcdefgha";

    function fromMemory(bytes memory m) public returns (bytes32) {
        return bytes32(m);
    }
    function fromCalldata(bytes calldata c) external returns (bytes32) {
        return bytes32(c);
    }
    function fromStorage() external returns (bytes32) {
        return bytes32(s);
    }
    function fromSlice(bytes calldata c) external returns (bytes32) {
        return bytes32(c[0:33]);
    }
}
// ----
// fromMemory(bytes): 0x20, 33, "abcdefghabcdefghabcdefghabcdefgh", "a" -> "abcdefghabcdefghabcdefghabcdefgh"
// fromCalldata(bytes): 0x20, 33, "abcdefghabcdefghabcdefghabcdefgh", "a" -> "abcdefghabcdefghabcdefghabcdefgh"
// fromStorage() -> "abcdefghabcdefghabcdefghabcdefgh"
// fromSlice(bytes): 0x20, 33, "abcdefghabcdefghabcdefghabcdefgh", "a" -> "abcdefghabcdefghabcdefghabcdefgh"
