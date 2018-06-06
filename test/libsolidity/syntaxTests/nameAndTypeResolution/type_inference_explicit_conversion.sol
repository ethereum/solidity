contract test {
    function f() public returns (int256 r) { var x = int256(uint32(2)); return x; }
}
// ----
// Warning: (61-66): Use of the "var" keyword is deprecated.
// Warning: (20-99): Function state mutability can be restricted to pure
