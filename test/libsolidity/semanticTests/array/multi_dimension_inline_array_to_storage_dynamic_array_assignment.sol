pragma abicoder v2;

contract C {
    uint8[] array1 = [1,2,3,4,5,6,7,8,9,10,11,12,13, 14, 15, 16];
    uint8[][] array2 = [[1, 2, 3],[4, 5],[6,7,8,9,10,11,12], [13,14,15,16]];
    uint8[][][] array3 = [[[1, 2, 3],[4, 5],[6]], [[7, 8, 9, 10,11],[12, 13,14,15,16]]];


    function test1() public returns (uint8[] memory) {
        return array1;
    }

    function test2() public returns (uint8[][] memory) {
        return array2;
    }

    function test3() public returns (uint8[][][] memory) {
        return array3;
    }
}

// ====
// EVMVersion: >=constantinople
// compileToEwasm: also
// compileViaYul: also
// ----
// test1() -> 0x20, 0x10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0x10
// test2() -> 0x20, 4, 0x80, 0x0100, 0x0160, 0x0260, 3, 1, 2, 3, 2, 4, 5, 7, 6, 7, 8, 9, 10, 11, 12, 4, 13, 14, 15, 0x10
// test3() -> 0x20, 0x02, 0x40, 0x01e0, 0x03, 0x60, 0xe0, 0x0140, 3, 1, 2, 3, 2, 4, 5, 1, 6, 2, 0x40, 0x0100, 5, 7, 8, 9, 10, 11, 5, 12, 13, 14, 15, 0x10
