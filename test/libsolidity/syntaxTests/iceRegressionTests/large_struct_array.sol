// Used to cause ICE because of a too strict assert
pragma abicoder               v2;
contract C {
    struct S { uint a; T[222222222222222222222222222] sub; }
    struct T { uint[] x; }
    function f() public returns (uint, S memory) {
    }
}
// ----
// TypeError 1534: (226-234): Type too large for memory.
