contract A {
    function f() {
        uint x = 3;
        uint y = 1;
        uint z = (x > y) ? x : y;
        uint w = x > y ? x : y;
    }
}
// ----
// Warning: (17-143): No visibility specified. Defaulting to "public". 
// Warning: (80-86): Unused local variable.
// Warning: (114-120): Unused local variable.
// Warning: (17-143): Function state mutability can be restricted to pure
