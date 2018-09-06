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
// Warning: (68-71): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (56-82): Function state mutability can be restricted to pure
