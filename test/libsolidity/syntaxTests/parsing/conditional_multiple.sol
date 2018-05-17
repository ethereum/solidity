contract A {
    function f() {
        uint x = 3 < 0 ? 2 > 1 ? 2 : 1 : 7 > 2 ? 7 : 6;
    }
}
// ----
// Warning: (17-93): No visibility specified. Defaulting to "public". 
// Warning: (40-46): Unused local variable.
// Warning: (17-93): Function state mutability can be restricted to pure
