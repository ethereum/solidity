contract D {
    uint x;
    function f() public { x = 2; }
}
contract C is D {
    function f() public {}
}
