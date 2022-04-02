contract test {
    function e() external { }
    function f() public pure { uint e; e = 0; }
}
// ----
// Warning 8760: (77-83='uint e'): This declaration has the same name as another declaration.
