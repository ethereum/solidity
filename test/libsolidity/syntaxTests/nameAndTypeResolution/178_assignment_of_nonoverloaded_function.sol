contract test {
    function f(uint a) public returns (uint) { return 2 * a; }
    function g() public returns (uint) { function (uint) returns (uint) x = f; return x(7); }
}
// ----
// Warning: (20-78): Function state mutability can be restricted to pure
