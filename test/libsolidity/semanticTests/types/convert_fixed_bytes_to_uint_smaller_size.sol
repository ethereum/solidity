contract Test {
    function bytesToUint(bytes4 s) public returns (uint16 h) {
        return uint16(uint32(s));
    }
}
// ====
// compileViaYul: also
// ----
// bytesToUint(bytes4): "abcd" -> 0x6364
