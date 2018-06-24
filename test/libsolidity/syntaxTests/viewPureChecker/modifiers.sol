contract D {
    uint x;
    modifier purem(uint) { _; }
    modifier viewm(uint) { uint a = x; _; a; }
    modifier nonpayablem(uint) { x = 2; _; }
}
contract C is D {
    function f() purem(0) pure public {}
    function g() viewm(0) view public {}
    function h() nonpayablem(0) public {}
    function i() purem(x) view public {}
    function j() viewm(x) view public {}
    function k() nonpayablem(x) public {}
    function l() purem(x = 2) public {}
    function m() viewm(x = 2) public {}
    function n() nonpayablem(x = 2) public {}
}
