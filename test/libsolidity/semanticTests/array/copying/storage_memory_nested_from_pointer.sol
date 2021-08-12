contract C {
    uint72[5][] a;

    function f() public returns (uint72, uint72, uint72, uint72, uint72, uint72, uint72) {
        for (uint i = 0; i < 4; i++)
            a.push();
        a[0][0] = 1;
        a[0][3] = 2;
        a[1][1] = 3;
        a[1][4] = 4;
        a[2][0] = 5;
        a[3][2] = 6;
        a[3][3] = 7;
        uint72[5][] storage a_ = a;
        uint72[5][] memory m = a_;
        return (m[0][0], m[0][3], m[1][1], m[1][4], m[2][0], m[3][2], m[3][3]);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 2, 3, 4, 5, 6, 7
// gas irOptimized: 207164
// gas legacy: 212330
// gas legacyOptimized: 211455
