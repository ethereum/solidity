contract Test {
    uint128[13] unused;
    uint256[][] a;
    uint256[4][] b;
    uint256[2][3] c;

    function test() external returns (uint256) {
        uint256[][] memory m = new uint256[][](2);
        m[0] = new uint256[](3);
        m[0][0] = 7; m[0][1] = 8; m[0][2] = 9;
        m[1] = new uint256[](4);
        m[1][1] = 7; m[1][2] = 8; m[1][3] = 9;
        a = m;
        return a[0][0] + a[0][1] + a[1][3];
    }

    function test1() external returns (uint256) {
        uint256[2][] memory m = new uint256[2][](1);
        m[0][0] = 1; m[0][1] = 2;
        b = m;
        return b[0][0] + b[0][1];
    }

    function test2() external returns (uint256) {
        uint256[2][2] memory m;
        m[0][0] = 1; m[1][1] = 2; m[0][1] = 3;
        c = m;
        return c[0][0] + c[1][1] + c[0][1];
    }

    function test3() external returns (uint256) {
        uint256[2][3] memory m;
        m[0][0] = 7; m[1][0] = 8; m[2][0] = 9;
        m[0][1] = 7; m[1][1] = 8; m[2][1] = 9;
        a = m;
        return a[0][0] + a[1][0] + a[2][1];
    }
}
// ----
// test() -> 24
// gas irOptimized: 226694
// gas legacy: 227133
// gas legacyOptimized: 226547
// test1() -> 3
// test2() -> 6
// test3() -> 24
// gas irOptimized: 133254
// gas legacy: 134295
// gas legacyOptimized: 133383
