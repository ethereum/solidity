contract C {
    struct S { uint a; S[][][] sub; }
    struct T { S s; }
    function f() public pure returns (uint x, T t) {
    }
}
// ----
// TypeError: (119-122): Internal or recursive type is not allowed for public or external functions.
