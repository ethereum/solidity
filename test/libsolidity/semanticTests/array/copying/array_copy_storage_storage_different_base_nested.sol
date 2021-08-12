contract c {
    uint48[5][2] data1;
    uint120[6][3] data2;

    function test() public returns (uint256 x, uint120 y) {
        data2[0][0] = 11;
        data2[1][0] = 22;
        data2[2][0] = 33;

        data1[0][0] = 0;
        data1[0][1] = 1;
        data1[0][2] = 2;
        data1[0][3] = 3;
        data1[0][4] = 4;
        data1[1][0] = 3;
        data2 = data1;
        assert(data1[0][1] == data2[0][1]);
        x = data2.length;
        y = data2[0][4];
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 3, 4
// gas irOptimized: 190510
// gas legacy: 195353
// gas legacyOptimized: 192441
