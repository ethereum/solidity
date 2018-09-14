contract A {
    fixed40x10 storeMe;
    function f(ufixed x, fixed32x8 y) public {
        ufixed8x1 a;
        fixed b;
    }
}
// ----
// Warning: (52-60): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (62-73): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (92-103): Unused local variable.
// Warning: (113-120): Unused local variable.
// Warning: (41-127): Function state mutability can be restricted to pure
