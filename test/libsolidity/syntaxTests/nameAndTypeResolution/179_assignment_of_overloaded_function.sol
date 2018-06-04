contract test {
    function f() public returns (uint) { return 1; }
    function f(uint a) public returns (uint) { return 2 * a; }
    function g() public returns (uint) { var x = f; return x(7); }
}
// ----
// Warning: (173-178): Use of the "var" keyword is deprecated.
// TypeError: (181-182): No matching declaration found after variable lookup.
