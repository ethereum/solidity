contract Test {
    function bytesToUint(bytes4 s) public returns (uint64 h) {
        return uint64(uint32(s));
    }
}
// ----
// bytesToUint(bytes4): "abcd" -> 0x61626364
