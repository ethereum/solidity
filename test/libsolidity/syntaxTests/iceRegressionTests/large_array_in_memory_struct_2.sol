contract C {
    struct R { uint[10][10] y; }
    struct S { uint a; uint b; R d; uint[20][20][2999999999999999999999999990] c; }
    function f() public pure {
        C.S memory y;
        C.S[10] memory z;
        y.a < 2;
        z; y;
    }
}
// ----
// TypeError: (169-181): Type too large for memory.
