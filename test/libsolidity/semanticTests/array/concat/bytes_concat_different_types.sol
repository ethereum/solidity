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
    function fixedBytesParam(bytes16 b1, bytes15 b2, bytes31 b3) public returns (
        bytes memory,
        bytes memory,
        bytes memory,
        bytes memory
    ) {
        return (
            bytes.concat(b1, b2),
            bytes.concat(b1, b3),
            bytes.concat(b1, "bcdef"),
            bytes.concat(b1, s)
        );
    }
    function fixedBytesParam2(bytes calldata c, bytes6 b1, bytes6 b2) public returns (bytes memory, bytes memory) {
        return (
            bytes.concat(s, b1, c),
            bytes.concat(b1, c, b2)
        );
    }
}
// ====
// revertStrings: debug
// ----
// f(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// g(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 66, "abcdabcdabcdabcdabcdabcdabcdabcd", "abcdefghabcdefghabcdefghabcdefgh", "ab"
// h(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// j(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 42, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdefbcdef"
// k(bytes,string): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// slice(bytes): 0x20, 4, "abcd" -> 0x20, 4, "abcd"
// strParam(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 37, "abcdabcdabcdabcdabcdabcdabcdabcd", "bcdef"
// fixedBytesParam(bytes16,bytes15,bytes31):
//  "aabbccddeeffgghh",
//  "abcdefghabcdefg",
//  "0123456789012345678901234567890" ->
//  0x80, 0xc0, 0x120, 0x160,
//  31, "aabbccddeeffgghhabcdefghabcdefg",
//  47, "aabbccddeeffgghh0123456789012345", "678901234567890",
//  21, "aabbccddeeffgghhbcdef",
//  21, "aabbccddeeffgghhbcdef"
// fixedBytesParam2(bytes,bytes6,bytes6): 0x60, left(0x010203040506), left(0x0708090A0B0C), 20, left(0x1011121314151617181920212223242526272829) ->
//   0x40, 0x80,
//   31, left(0x62636465660102030405061011121314151617181920212223242526272829),
//   32, 0x01020304050610111213141516171819202122232425262728290708090A0B0C
// fixedBytesParam2(bytes,bytes6,bytes6): 0x60, left(0x01), left(0x02), 5, left(0x03) ->
//   0x40, 0x80,
//   16, left(0x6263646566010000000000030000000000),
//   17, left(0x010000000000030000000002000000000000)
