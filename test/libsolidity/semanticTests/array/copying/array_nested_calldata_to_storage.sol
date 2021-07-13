pragma abicoder               v2;

contract c {
    uint256[][] a1;
    uint256[][2] a2;
    uint256[2][] a3;
    uint256[2][2] a4;

    function test1(uint256[][] calldata c) external returns (uint256, uint256) {
        a1 = c;
        assert(a1[0][0] == c[0][0]);
        assert(a1[0][1] == c[0][1]);
        return (a1.length, a1[0][0] + a1[1][1]);
    }

    function test2(uint256[][2] calldata c) external returns (uint256, uint256) {
        a2 = c;
        assert(a2[0][0] == c[0][0]);
        assert(a2[0][1] == c[0][1]);
        return (a2[0].length, a2[0][0] + a2[1][1]);
    }

    function test3(uint256[2][] calldata c) external returns (uint256, uint256) {
        a3 = c;
        assert(a3[0][0] == c[0][0]);
        assert(a3[0][1] == c[0][1]);
        return (a3.length, a3[0][0] + a3[1][1]);
    }

    function test4(uint256[2][2] calldata c) external returns (uint256) {
        a4 = c;
        assert(a4[0][0] == c[0][0]);
        assert(a4[0][1] == c[0][1]);
        return (a4[0][0] + a4[1][1]);
    }
}
// ====
// compileViaYul: true
// ----
// test1(uint256[][]): 0x20, 2, 0x40, 0x40, 2, 23, 42 -> 2, 65
// gas irOptimized: 181308
// test2(uint256[][2]): 0x20, 0x40, 0x40, 2, 23, 42 -> 2, 65
// gas irOptimized: 157895
// test3(uint256[2][]): 0x20, 2, 23, 42, 23, 42 -> 2, 65
// gas irOptimized: 135108
// test4(uint256[2][2]): 23, 42, 23, 42 -> 65
// gas irOptimized: 111428
