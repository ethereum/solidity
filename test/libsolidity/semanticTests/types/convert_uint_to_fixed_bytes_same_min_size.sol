contract Test {
    function UintToBytes(uint8 h) public returns (bytes1 s) {
        return bytes1(h);
    }
}
// ----
// UintToBytes(uint8): 0x61 -> "a"
