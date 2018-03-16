contract A {
    function f(uint a) mod(2) public returns (uint r) { }
    modifier mod(uint a) { _; return 7; }
}
// ----
// TypeError: Return arguments not allowed.
