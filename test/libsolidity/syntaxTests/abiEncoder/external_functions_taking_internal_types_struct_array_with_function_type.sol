pragma abicoder v2;

contract C {
    struct S { function() internal a; }
    function f(S[2] memory) public {}
}
// ----
// TypeError 4103: (89-100='S[2] memory'): Internal type is not allowed for public or external functions.
