contract C {
    function f(bytes memory a, bytes memory b) public returns (bytes memory) {
        return bytes.concat(a, b);
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes,bytes): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
//
// f(bytes,bytes):
//   0x40, 0xa0, 64, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef"
//   ->
//   0x20, 69, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// f(bytes,bytes): 0x40, 0x80, 3, "abc", 3, "def" -> 0x20, 6, "abcdef"
//
// f(bytes,bytes):
//  0x40, 0xa0, 34, "abcdabcdabcdabcdabcdabcdabcdabcd", "ab", 30, "cdabcdabcdabcdabcdabcdabcdabcd"
//  ->
//  0x20, 64, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdabcdabcdabcdabcdabcdabcdabcd"
//
// f(bytes,bytes):
//  0x40, 0xa0, 34, "abcdabcdabcdabcdabcdabcdabcdabcd", "ab", 34, "cdabcdabcdabcdabcdabcdabcdabcdab", "cd"
//  ->
//  0x20, 68, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdabcdabcdabcdabcdabcdabcdabcd", "abcd"
//
// f(bytes,bytes):
//   0x40, 0x80, 3, "abc", 30, "dabcdabcdabcdabcdabcdabcdabcda"
//   ->
//   0x20, 33, "abcdabcdabcdabcdabcdabcdabcdabcd", "a"
