contract test {
    function f() public returns (uint) { return 1; }
    function f(uint a) public returns (uint) { return a; }
    function g() public returns (uint) { return f(3, 5); }
}
// ----
// TypeError: (176-177): No matching declaration found after argument-dependent lookup.
