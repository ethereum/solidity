contract A {
    function f() {
        uint x = true ? 1 : 0;
        uint y = false ? 0 : 1;
    }
}
// ----
// Warning: (17-100): No visibility specified. Defaulting to "public". 
// Warning: (40-46): Unused local variable.
// Warning: (71-77): Unused local variable.
// Warning: (17-100): Function state mutability can be restricted to pure
