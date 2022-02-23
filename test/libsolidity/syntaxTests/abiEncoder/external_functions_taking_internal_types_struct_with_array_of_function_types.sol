pragma abicoder v2;

contract C {
    struct S { function() internal[2] a; }
    function f(S memory) public {}
}
// ----
// TypeError 4103: (92-100): Internal type is not allowed for public or external functions.
