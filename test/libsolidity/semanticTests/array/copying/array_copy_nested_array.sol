contract c {
    uint256[4][] a;
    uint256[10][] b;
    uint256[][] c;

    function test(uint256[2][] calldata d) external returns (uint256) {
        a = d;
        b = a;
        c = b;
        return c[1][1] | c[1][2] | c[1][3] | c[1][4];
    }
}

// ====
// compileViaYul: also
// ----
// test(uint256[2][]): 32, 3, 7, 8, 9, 10, 11, 12 -> 10
// gas irOptimized: 690205
// gas legacy: 686268
// gas legacyOptimized: 685638
