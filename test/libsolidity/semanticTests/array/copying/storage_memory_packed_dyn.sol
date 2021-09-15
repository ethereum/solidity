contract C {
    uint8[] a;

    function f() public returns (uint8, uint8, uint8) {
        for (uint i = 0; i < 33; i++)
            a.push(7);
        a[0] = 2;
        a[16] = 3;
        a[32] = 4;
        uint8[] memory m = a;
        return (m[0], m[16], m[32]);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 2, 3, 4
// gas irOptimized: 114120
// gas legacy: 126449
// gas legacyOptimized: 120902
