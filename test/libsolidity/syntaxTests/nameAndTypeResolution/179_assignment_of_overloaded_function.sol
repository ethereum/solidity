contract test {
    function f() public returns (uint) { return 1; }
    function f(uint a) public returns (uint) { return 2 * a; }
    function g() public returns (uint) { function (uint) returns (uint) x = f; return x(7); }
}
// ----
// TypeError: (208-209): No matching declaration found after variable lookup.
