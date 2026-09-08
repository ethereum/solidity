// `delete b[i]` on an element of a `bytes`/`string` in memory must clear exactly one byte, like `b[i] = 0`
contract C {
    function del0() external pure returns (bytes memory) {
        bytes memory data = hex"0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20";
        delete data[0];
        return data;
    }

    function delShort() external pure returns (bytes memory) {
        bytes memory data = hex"0102030405";
        delete data[1];
        return data;
    }

    // A `string memory` has no `[]`, but `bytes(s)` aliases the same memory
    function delStringViaBytes() external pure returns (string memory) {
        string memory s = "abcdefgh";
        delete bytes(s)[2];
        return s;
    }
}
// ----
// del0() -> 0x20, 0x20, 0x0002030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20
// delShort() -> 0x20, 5, left(0x0100030405)
// delStringViaBytes() -> 0x20, 8, "ab\x00defgh"
