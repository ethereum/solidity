contract C {
    uint8[][][] src1 = new uint8[][][](2);
    uint8[][2][] src2 = new uint8[][2][](1);
    uint8[2][][] src3 = new uint8[2][][](2);

    uint8[][][1] dst1;
    uint8[][2][] dst2;
    uint8[2][] dst3;

    constructor() {
        src1[1] = new uint8[][](2);
        src1[1][0] = [3, 4];
        src1[1][1] = [5, 6];

        src2[0][0] = [3, 4];
        src2[0][1] = [5, 6];

        src3[0] = new uint8[2][](1);
        src3[0][0] = [17, 23];
        src3[1] = new uint8[2][](1);
        src3[1][0] = [19, 31];
    }

    function test1() public {
	dst1[0] = src1[1];
        require(dst1[0].length == 2);
	require(dst1[0][0][0] == src1[1][0][0]);
        require(dst1[0][1][1] == src1[1][1][1]);
    }

    function test2() public {
        dst2.push();
	dst2[0][0] = src2[0][0];
        dst2[0][1] = src2[0][1];
        require(dst2[0][0][0] == src2[0][0][0]);
        require(dst2[0][0][1] == src2[0][0][1]);
    }

    function test3() public {
        dst3 = src3[1];
        require(dst3[0][0] == src3[1][0][0]);
        require(dst3[0][0] == src3[1][0][0]);
    }
}

// ----
// test1()
// gas irOptimized: 147595
// gas legacy: 148507
// gas legacyOptimized: 148336
// test2()
// gas irOptimized: 145663
// gas legacy: 146311
// gas legacyOptimized: 146106
// test3()
