pragma abicoder v2;

contract C {
    function test1(uint8[][][] calldata _a) public returns (uint8[][] memory) {
        return _a[1];
    }

    function test2(uint8[][1][] calldata _a) public returns (uint8[][1] memory) {
        return _a[0];
    }

    function test3(uint8[2][][2] calldata _a) public returns (uint8[2][] memory) {
        return _a[0];
    }

    function test4(uint16[][] calldata _a) public returns (uint16[][] memory) {
        uint16[][][] memory tmp = new uint16[][][](2);
        tmp[1] = _a;
        return tmp[1];
    }

    function test5(uint32[][2] calldata _a) public returns (uint32[][2] memory) {
        uint32[][2][] memory tmp = new uint32[][2][](1);
        tmp[0] = _a;
        return tmp[0];
    }

    function test6(uint32[2][] calldata _a) public returns (uint32[2][] memory) {
        uint32[2][][] memory tmp = new uint32[2][][](1);
        tmp[0] = _a;
        return tmp[0];
    }
}

// ----
// test1(uint8[][][]): 0x20, 2, 0x40, 0x60, 0, 2, 0x40, 0x80, 1, 7, 2, 8, 9 -> 0x20, 2, 0x40, 0x80, 1, 7, 2, 8, 9
// test2(uint8[][1][]): 0x20, 2, 0x40, 0xe0, 0x20, 3, 12, 13, 14, 0x20, 3, 15, 16, 17 -> 0x20, 0x20, 3, 12, 13, 14
// test3(uint8[2][][2]): 0x20, 0x40, 0xa0, 1, 7, 7, 2, 8, 8, 0x09, 9 -> 0x20, 1, 7, 7
// test4(uint16[][]): 0x20, 2, 0x40, 0x80, 1, 7, 2, 8, 9 -> 0x20, 2, 0x40, 0x80, 1, 7, 2, 8, 9
// test5(uint32[][2]): 0x20, 0x40, 0x80, 1, 7, 2, 8, 9 -> 0x20, 0x40, 0x80, 1, 7, 2, 8, 9
// test6(uint32[2][]): 0x20, 2, 5, 6, 7, 8 -> 0x20, 2, 5, 6, 7, 8
