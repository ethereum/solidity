contract A {
    function f(uint a) mod(2) public returns (uint r) { }
    modifier mod(uint a) { _; return 7; }
}
// ----
// TypeError 7552: (101-109='return 7'): Return arguments not allowed.
