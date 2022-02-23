pragma abicoder v2;

contract C {
    bytes10[] s;
    function f() external returns (uint256, bytes10, bytes10, bytes10) {
        bytes4[] memory m = new bytes4[](3);
        m[0] = "abcd";
        m[1] = "bcde";
        m[2] = "cdef";
        s = m;
        return (s.length, s[0], s[1], s[2]);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 3, "abcd", "bcde", "cdef"
