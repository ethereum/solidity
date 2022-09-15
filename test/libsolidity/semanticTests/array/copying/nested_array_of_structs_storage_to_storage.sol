pragma abicoder v2;

contract C {
    struct S {
        uint8 x;
        uint8 y;
    }

    S[][] src1;
    S[][1] src2;
    S[1][] src3;

    S[][] dst1;
    S[][1] dst2;
    S[1][] dst3;

    constructor() {
        src1 = new S[][](1);
        src1[0].push(S({x: 3, y: 7}));
        src1[0].push(S({x: 11, y: 13}));

        src2[0].push(S({x: 3, y: 7}));
        src2[0].push(S({x: 11, y: 13}));
        src2[0].push(S({x: 17, y: 19}));

        src3.push([S({x: 3, y: 7})]);
        src3.push([S({x: 11, y: 13})]);
    }

    function test1() public {
        dst1 = src1;

        require(dst1.length == 1);
        require(dst1[0][0].x == src1[0][0].x);
        require(dst1[0][0].y == src1[0][0].y);
        require(dst1[0][1].x == src1[0][1].x);
        require(dst1[0][1].y == src1[0][1].y);
    }

    function test2() public {
        dst2 = src2;

        require(dst2[0].length == 3);
        require(dst2[0][0].x == src2[0][0].x);
        require(dst2[0][0].y == src2[0][0].y);
        require(dst2[0][1].x == src2[0][1].x);
        require(dst2[0][1].y == src2[0][1].y);
        require(dst2[0][2].x == src2[0][2].x);
        require(dst2[0][2].y == src2[0][2].y);
    }

    function test3() public {
        dst3 = src3;

        require(dst3.length == 2);
        require(dst3[0][0].x == src3[0][0].x);
        require(dst3[0][0].y == src3[0][0].y);
        require(dst3[1][0].x == src3[1][0].x);
        require(dst3[1][0].y == src3[1][0].y);
    }
}

// ====
// compileViaYul: true
// ----
// test1()
// gas irOptimized: 123279
// test2()
// gas irOptimized: 123073
// test3()
