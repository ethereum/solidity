contract B {
    function f() mod(x) pure public { uint x = 7; }
    modifier mod(uint a) { if (a > 0) _; }
}
