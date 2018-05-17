contract A {
    function f() {
        uint x = 3 > 0 ? 3 : 0;
        uint y = (3 > 0) ? 3 : 0;
    }
}
// ----
// Warning: (17-103): No visibility specified. Defaulting to "public". 
// Warning: (40-46): Unused local variable.
// Warning: (72-78): Unused local variable.
// Warning: (17-103): Function state mutability can be restricted to pure
