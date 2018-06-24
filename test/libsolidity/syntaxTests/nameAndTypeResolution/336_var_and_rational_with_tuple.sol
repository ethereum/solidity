contract test {
    function f() public {
        var (a, b) = (.5, 1/3);
        a; b;
    }
}
// ----
// Warning: (55-56): Use of the "var" keyword is deprecated.
// Warning: (58-59): Use of the "var" keyword is deprecated.
// Warning: (50-72): The type of this variable was inferred as ufixed8x1. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (50-72): The type of this variable was inferred as ufixed256x77. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (20-93): Function state mutability can be restricted to pure
