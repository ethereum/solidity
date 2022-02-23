pragma abicoder               v2;

contract C {
    struct S { uint a; S[][][] sub; }
    struct T { S s; }
    function f() public pure returns (uint x, T memory t) {
    }
}
// ----
// TypeError 4103: (154-164): Recursive type not allowed for public or external contract functions.
