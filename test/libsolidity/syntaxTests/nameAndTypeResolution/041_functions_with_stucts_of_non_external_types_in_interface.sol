pragma experimental ABIEncoderV2;

contract C {
    struct S { function() internal a; }
    function f(S memory) public {}
}
// ----
// TypeError: (103-111): Internal type is not allowed for public or external functions.
