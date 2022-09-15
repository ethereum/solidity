pragma abicoder v2;

contract C {
    uint8[][][] a1 = new uint8[][][](2);
    uint8[][][2] a2;
    uint8[][2][] a3 = new uint8[][2][](1);
    uint8[2][][] a4 = new uint8[2][][](2);

    constructor() {
        a1[1] = new uint8[][](2);
        a1[1][0] = [3, 4];
        a1[1][1] = [5, 6];

        a2[0] = new uint8[][](2);
        a2[0][0] = [6, 7];
        a2[0][1] = [8, 9];
        a2[1] = new uint8[][](2);
        a2[1][0] = [10, 11];

        a3[0][0] = [3, 4];
        a3[0][1] = [5, 6];

        a4[0] = new uint8[2][](1);
        a4[0][0] = [17, 23];
        a4[1] = new uint8[2][](1);
        a4[1][0] = [19, 31];
    }

    function test1() public returns (uint8[][] memory) {
        return a1[1];
    }

    function test2() public returns (uint8[][] memory) {
        return a2[0];
    }

    function test3() public returns (uint8[][2] memory) {
        return a3[0];
    }

    function test4() public returns (uint8[2][] memory) {
        return a4[1];
    }

    function test5() public returns (uint8[][][] memory) {
        uint8[][][] memory tmp = new uint8[][][](3);
        tmp[1] = a1[1];
        return tmp;
    }

    function test6() public returns (uint8[][2][] memory) {
        uint8[][2][] memory tmp = new uint8[][2][](2);
        tmp[0] = a3[0];
        return tmp;
    }

    function test7() public returns (uint8[2][][] memory) {
        uint8[2][][] memory tmp = new uint8[2][][](1);
        tmp[0] = a4[0];
        return tmp;
    }
}

// ----
// test1() -> 0x20, 2, 0x40, 0xa0, 2, 3, 4, 2, 5, 6
// test2() -> 0x20, 2, 0x40, 0xa0, 2, 6, 7, 2, 8, 9
// test3() -> 0x20, 0x40, 0xa0, 2, 3, 4, 2, 5, 6
// test4() -> 0x20, 1, 19, 31
// test5() -> 0x20, 3, 0x60, 0x80, 0x01a0, 0, 2, 0x40, 0xa0, 2, 3, 4, 2, 5, 6, 0
// test6() -> 0x20, 2, 0x40, 0x0140, 0x40, 0xa0, 2, 3, 4, 2, 5, 6, 0x40, 0x60, 0, 0
// test7() -> 0x20, 1, 0x20, 1, 17, 23
