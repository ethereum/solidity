contract C {
    function g() pure public { g(); }
    function f() view public returns (uint) { f(); g(); }
    function h() public { h(); g(); f(); }
    function i() payable public { i(); h(); g(); f(); }
}
