pragma abicoder               v2;

contract C {
    struct S1 { function() external a; }
    struct S2 { bytes24 a; }
    function f(S1 memory) public pure {}
    function f(S2 memory) public pure {}
}
// ----
