contract C {
    function f(bytes memory a, bytes memory b, bytes memory c) public returns (bytes memory) {
        return bytes.concat(bytes.concat(a, b), c);
    }
}
// ----
// f(bytes,bytes,bytes): 0x60, 0x60, 0x60, 2, "ab" -> 0x20, 6, "ababab"
