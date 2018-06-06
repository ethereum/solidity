contract C {
    uint a;
}
contract Test {
    address a;
    function g (C c) public {}
    function internalCall() public {
        g(a);
    }
}
// ----
// TypeError: (136-137): Invalid type for argument in function call. Invalid implicit conversion from address to contract C requested.
