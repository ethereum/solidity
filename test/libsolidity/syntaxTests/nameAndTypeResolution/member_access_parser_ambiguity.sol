contract C {
    struct R { uint[10][10] y; }
    struct S { uint a; uint b; uint[20][20][20] c; R d; }
    S data;
    function f() public {
        C.S x = data;
        C.S memory y;
        C.S[10] memory z;
        C.S[10];
        y.a = 2;
        x.c[1][2][3] = 9;
        x.d.y[2][2] = 3;
    }
}
// ----
// Warning: (150-155): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// Warning: (194-210): Unused local variable.
