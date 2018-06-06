contract test {
    function f() public {
        var a = 0.12345678;
        var b = 12345678.352;
        var c = 0.00000009;
        a; b; c;
    }
}
// ----
// Warning: (50-55): Use of the "var" keyword is deprecated.
// Warning: (78-83): Use of the "var" keyword is deprecated.
// Warning: (108-113): Use of the "var" keyword is deprecated.
// Warning: (50-68): The type of this variable was inferred as ufixed24x8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (78-98): The type of this variable was inferred as ufixed40x3. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (108-126): The type of this variable was inferred as ufixed8x8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (20-150): Function state mutability can be restricted to pure
