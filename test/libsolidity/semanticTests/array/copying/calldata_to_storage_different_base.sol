pragma abicoder v2;

contract C {
    bytes10[] s;
    function f(bytes8[] calldata c) external returns (uint256, bytes10, bytes10, bytes10) {
        s = c;
        return (s.length, s[0], s[1], s[2]);
    }
}
// ----
// f(bytes8[]): 0x20, 3, "abcd", "bcde", "cdef" -> 3, "abcd", "bcde", "cdef"
