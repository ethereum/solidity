contract B {
    function f(uint x) mod(x) mod(2) public pure { }
    modifier mod(uint a) { if (a > 0) _; }
}
// ----
