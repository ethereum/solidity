contract test {
    function f() public {
        var x = 1/3;
    }
}
// ----
// Warning: (50-55): Use of the "var" keyword is deprecated.
// Warning: (50-61): The type of this variable was inferred as ufixed256x77. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (50-55): Unused local variable.
// Warning: (20-68): Function state mutability can be restricted to pure
