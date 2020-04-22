// Used to cause ICE because of a too strict assert
pragma experimental ABIEncoderV2;
contract C {
    struct S { uint a; T[222222222222222222222222222] sub; }
    struct T { uint[] x; }
    function f() public returns (uint, S memory) {
    }
}
// ----
// TypeError: (226-234): Type too large for memory.
