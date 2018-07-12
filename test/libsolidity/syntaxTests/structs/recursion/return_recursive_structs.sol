contract C {
    struct S { uint a; S[] sub; }
    function f() public pure returns (uint, S memory) {
    }
}
// ----
// TypeError: (91-92): Internal or recursive type is not allowed for public or external functions.
