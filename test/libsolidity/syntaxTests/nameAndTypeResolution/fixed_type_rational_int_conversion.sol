contract test {
    function f() public {
        fixed c = 3;
        ufixed d = 4;
        c; d;
    }
}
// ----
// Warning: (20-104): Function state mutability can be restricted to pure
