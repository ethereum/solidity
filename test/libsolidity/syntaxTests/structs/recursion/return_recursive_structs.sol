contract C {
    struct S { uint a; S[] sub; }
    function f() public pure returns (uint, S memory) {
    }
}
// ----
// TypeError: (91-99): Recursive type not allowed for public or external contract functions.
