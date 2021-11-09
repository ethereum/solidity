contract C {
    uint8[33] a;
    uint32[9] b;
    uint120[3] c;

    function f() public returns (uint8, uint32, uint120) {
        a[32] = 1; a[31] = 2; a[30] = 3;
        b[0] = 1; b[1] = 2; b[2] = 3;
        c[2] = 3; c[1] = 1;
        return (a[32], b[1], c[2]);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1, 2, 3
// gas irOptimized: 132298
// gas legacy: 134619
// gas legacyOptimized: 131940
