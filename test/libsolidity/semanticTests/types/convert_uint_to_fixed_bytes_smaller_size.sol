contract Test {
    function uintToBytes(uint32 h) public returns (bytes2 s) {
        return bytes2(uint16(h));
    }
}
// ====
// compileViaYul: also
// ----
// uintToBytes(uint32):  0x61626364 -> "cd"
