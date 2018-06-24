contract A {
    fixed40x40 storeMe;
    function f(ufixed x, fixed32x32 y) {
        ufixed8x8 a;
        fixed b;
    }
}
// ----
// Warning: (41-121): No visibility specified. Defaulting to "public". 
// Warning: (52-60): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (62-74): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (86-97): Unused local variable.
// Warning: (107-114): Unused local variable.
// Warning: (41-121): Function state mutability can be restricted to pure
