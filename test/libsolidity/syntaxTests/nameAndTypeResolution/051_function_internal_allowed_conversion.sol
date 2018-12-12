contract C {
    uint a;
}
contract Test {
    C a;
    function g (C c) public {}
    function internalCall() public {
        g(a);
    }
}
// ----
