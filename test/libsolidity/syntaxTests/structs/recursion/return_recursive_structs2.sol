contract C {
    struct S { uint a; S[2][] sub; }
    function f() public pure returns (uint, S memory) {
    }
}
// ----
// TypeError: (94-95): Internal or recursive type is not allowed for public or external functions.
