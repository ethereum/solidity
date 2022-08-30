contract Test {
    function UintToBytes(uint16 h) public returns (bytes8 s) {
        return bytes8(uint64(h));
    }
}
// ====
// compileToEwasm: also
// ----
// UintToBytes(uint16): 0x6162 -> "\x00\x00\x00\x00\x00\x00ab"
