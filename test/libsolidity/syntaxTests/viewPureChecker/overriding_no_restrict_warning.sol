contract D {
    uint x;
    function f() virtual public { x = 2; }
}
contract C is D {
    function f() public override {}
}
