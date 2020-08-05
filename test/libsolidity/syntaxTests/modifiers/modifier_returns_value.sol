contract A {
    function f(uint a) mod(2) public returns (uint r) { }
    modifier mod(uint a) { _; return 7; }
}
// ----
// TypeError 7552: (101-109): Return arguments not allowed.
