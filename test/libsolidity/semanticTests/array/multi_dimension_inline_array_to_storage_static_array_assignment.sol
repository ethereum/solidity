contract C {
    uint16[1][16] array1 = [[1], [2], [3], [4], [5], [6], [7], [8], [9], [10], [11], [12], [13], [14], [15], [16]];
    uint16[4][2] array2 = [[1, 2, 3, 4], [5, 6, 7, 8]];
    uint8[4][2][2] array3 = [[[1, 2, 3, 4], [5, 6, 7, 8]], [[9, 10, 11, 12], [13, 14, 15, 16]]];

    function test1() public returns (uint16[1][16] memory) {
        return array1;
    }

    function test2() public returns (uint16[4][2] memory) {
        return array2;
    }

    function test3() public returns (uint8[4][2][2] memory) {
        return array3;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test1() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
// test2() -> 1, 2, 3, 4, 5, 6, 7, 8
// test3() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
