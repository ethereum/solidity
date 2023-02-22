pragma abicoder v2;

contract C {
    uint8[][][] src1 = new uint8[][][](2);
    uint8[][][2] src2;
    uint8[][2][] src3 = new uint8[][2][](1);
    uint8[2][][] src4 = new uint8[2][][](2);

    uint8[][] dst1;
    uint8[][] dst2;
    uint8[][2] dst3;
    uint8[][] dst4;

    constructor() {
        src1[1] = new uint8[][](2);
        src1[1][0] = [3, 4];
        src1[1][1] = [5, 6];

        src2[0] = new uint8[][](2);
        src2[0][0] = [6, 7];
        src2[0][1] = [8, 9];
        src2[1] = new uint8[][](2);
        src2[1][0] = [10, 11];

        src3[0][0] = [3, 4];
        src3[0][1] = [5, 6];

        src4[0] = new uint8[2][](1);
        src4[0][0] = [17, 23];
        src4[1] = new uint8[2][](1);
        src4[1][0] = [19, 31];
    }

    function test1() public {
        dst1 = src1[1];

        require(dst1.length == 2);
        require(dst1[0][0] == src1[1][0][0]);
        require(dst1[0][1] == src1[1][0][1]);
        require(dst1[1][0] == src1[1][1][0]);
        require(dst1[1][1] == src1[1][1][1]);
    }

    function test2() public {
        dst2 = src2[0];

        require(dst2.length == 2);
        require(dst2[0][0] == src2[1][0][0]);
        require(dst2[0][1] == src2[1][0][1]);
        require(dst2[1][0] == src2[1][1][0]);
        require(dst2[1][1] == src2[1][1][1]);
    }

    function test3() public {
        dst3 = src3[0];
        require(dst3[0][0] == src3[0][0][0]);
        require(dst3[0][1] == src3[0][0][1]);
        require(dst3[1][0] == src3[0][1][0]);
        require(dst3[1][1] == src3[0][1][1]);
    }

    function test4() public {
        dst4 = src4[1];
        require(dst4.length == 2);
        require(dst4[0][0] == src4[0][0][0]);
        require(dst4[0][1] == src4[0][0][1]);
        require(dst4[1][0] == src4[0][1][0]);
        require(dst4[1][1] == src4[0][1][1]);
    }
}
// ----
// test1() ->
// gas irOptimized: 150508
// gas legacy: 150949
// gas legacyOptimized: 150906
// test2() -> FAILURE
// gas irOptimized: 150385
// gas legacy: 150673
// gas legacyOptimized: 150576
// test3() ->
// gas irOptimized: 123776
// gas legacy: 125333
// gas legacyOptimized: 125127
// test4() -> FAILURE
