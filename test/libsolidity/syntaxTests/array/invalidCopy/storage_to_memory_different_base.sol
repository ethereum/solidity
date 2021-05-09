pragma abicoder v2;

contract C {
    bytes8[] s;
    function f() external returns (uint256, bytes10, bytes10, bytes10) {
        s.push("abcd");
        s.push("bcde");
        s.push("cdef");
        bytes10[] memory m = s;
        return (m.length, m[0], m[1], m[2]);
    }
}
// ----
// TypeError 9574: (203-225): Type bytes8[] storage ref is not implicitly convertible to expected type bytes10[] memory.
