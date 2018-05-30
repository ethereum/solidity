contract test {
    function f(uint a) public returns (uint) { return 2 * a; }
    function g() public returns (uint) { var x = f; return x(7); }
}
// ----
// Warning: (120-125): Use of the "var" keyword is deprecated.
// Warning: (20-78): Function state mutability can be restricted to pure
