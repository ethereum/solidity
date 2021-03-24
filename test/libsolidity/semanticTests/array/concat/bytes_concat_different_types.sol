contract C {
    bytes s = "bcdef";

    function f(bytes memory a) public returns (bytes memory) {
        return bytes.concat(a, "bcdef");
    }
    function g(bytes calldata a) public returns (bytes memory) {
        return bytes.concat(a, "abcdefghabcdefghabcdefghabcdefghab");
    }
    function h(bytes calldata a) public returns (bytes memory) {
        return bytes.concat(a, s);
    }
    function j(bytes calldata a) public returns (bytes memory) {
        bytes storage ref = s;
        return bytes.concat(a, ref, s);
    }
    function k(bytes calldata a, string memory b) public returns (bytes memory) {
        return bytes.concat(a, bytes(b));
    }
    function slice(bytes calldata a) public returns (bytes memory) {
        require(a.length > 2, "");
        return bytes.concat(a[:2], a[2:]);
    }
    function strParam(string calldata a) public returns (bytes memory) {
        return bytes.concat(bytes(a), "bcdef");
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// g(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 66, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdefghabcdefghabcdefghabcdefgh", "ab"
// h(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// j(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 42, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdefbcdef"
// k(bytes, string): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// slice(bytes): 0x20, 4, "abcd" -> 0x20, 4, "abcd"
// strParam(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
