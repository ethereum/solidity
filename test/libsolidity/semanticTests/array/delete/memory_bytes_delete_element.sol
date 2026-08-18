// `delete b[i]` on an element of a `bytes`/`string` in memory must clear exactly one byte, like `b[i] = 0`
contract C {
    function del0() external pure returns (bytes memory) {
        bytes memory data = hex"0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20";
        delete data[0];
        return data;
    }

    // The buffer is 5 bytes, so a full-word write from index 1 runs off the end of the payload.
    function delShort() external pure returns (bytes memory) {
        bytes memory data = hex"0102030405";
        delete data[1];
        return data;
    }

    // A `string memory` has no `[]`, but `bytes(s)` aliases the same memory,
    // so the same defect corrupts a string in place
    function delStringViaBytes() external pure returns (string memory) {
        string memory s = "abcdefgh";
        delete bytes(s)[2];
        return s;
    }
}
// ====
// compileViaYul: false
// ----
// del0() -> 0x20, 0x20, 0
// delShort() -> 0x20, 5, left(0x0100000000)
// delStringViaBytes() -> 0x20, 8, "ab"
