contract c {
    uint256[9] data1;
    uint256[] data2;

    function test() public returns (uint256 x, uint256 y) {
        data1[8] = 4;
        data2 = data1;
        x = data2.length;
        y = data2[8];
    }
}

// ====
// compileViaYul: also
// ----
// test() -> 9, 4
// gas irOptimized: 123162
// gas legacy: 123579
// gas legacyOptimized: 123208
