contract test {
    function f() public returns (bool r) { return 1 >= 2; }
}
// ----
// Warning 2018: (20-75): Function state mutability can be restricted to pure
