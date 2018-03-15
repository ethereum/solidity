contract C {
    struct S { uint a; S[2][] sub; }
    function f() public pure returns (uint, S) {
    }
}
// ----
// TypeError: Internal or recursive type is not allowed for public or external functions.
