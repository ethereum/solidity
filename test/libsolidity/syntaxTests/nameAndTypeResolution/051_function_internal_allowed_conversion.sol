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
// Warning: (56-82): Function state mutability can be restricted to pure
