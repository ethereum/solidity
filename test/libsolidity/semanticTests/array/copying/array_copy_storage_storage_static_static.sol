contract c {
    uint256[40] data1;
    uint256[20] data2;

    function test() public returns (uint256 x, uint256 y) {
        data1[30] = 4;
        data1[2] = 7;
        data1[3] = 9;
        data2[3] = 8;
        data1 = data2;
        x = data1[3];
        y = data1[30]; // should be cleared
    }
}
// ----
// test() -> 8, 0
// gas irOptimized: 196278
// gas legacy: 194843
// gas legacyOptimized: 194281
