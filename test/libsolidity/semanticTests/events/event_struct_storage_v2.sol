pragma abicoder v2;
contract C {
    struct S { uint a; }
    event E(S);
    S s;
    function createEvent(uint x) public {
        s.a = x;
        emit E(s);
    }
}
// ----
// createEvent(uint256): 42 ->
// ~ emit E((uint256)): 0x2a
