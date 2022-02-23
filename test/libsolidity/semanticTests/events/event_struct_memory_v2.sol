pragma abicoder v2;
contract C {
    struct S { uint a; }
    event E(S);
    function createEvent(uint x) public {
        emit E(S(x));
    }
}
// ====
// compileViaYul: also
// ----
// createEvent(uint256): 42 ->
// ~ emit E((uint256)): 0x2a
