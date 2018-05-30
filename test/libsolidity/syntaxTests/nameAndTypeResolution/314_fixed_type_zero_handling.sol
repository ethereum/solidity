contract test {
    function f() public {
        fixed16x2 a = 0; a;
        ufixed32x1 b = 0; b;
    }
}
// ----
// Warning: (20-104): Function state mutability can be restricted to pure
