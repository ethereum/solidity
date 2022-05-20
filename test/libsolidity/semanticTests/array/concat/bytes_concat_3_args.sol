contract C {
    function f(bytes memory a, bytes memory b, bytes memory c) public returns (bytes memory) {
        return bytes.concat(a, b, c);
    }
}
// ----
// f(bytes,bytes,bytes): 0x60, 0xa0, 0xe0, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef", 3, "abc" -> 0x20, 40, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdefabc"
// f(bytes,bytes,bytes): 0x60, 0xa0, 0xe0, 3, "abc", 2, "de", 3, "fgh" -> 0x20, 8, "abcdefgh"
