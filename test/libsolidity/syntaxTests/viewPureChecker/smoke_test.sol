contract C {
    uint x;
    function g() pure public {}
    function f() view public returns (uint) { return block.timestamp; }
    function h() public { x = 2; }
    function i() payable public { x = 2; }
}
