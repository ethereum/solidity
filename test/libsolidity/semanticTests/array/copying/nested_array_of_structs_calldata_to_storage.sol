pragma abicoder v2;

contract C {
    struct S {
        uint8[] a;
        uint8[2] b;
    }

    S[][] s1;
    S[][1] s2;
    S[1][] s3;

    function test1(S[][] calldata _a) public returns (S[][] memory){
        s1 = _a;
        return s1;
    }

    function test2(S[][1] calldata _a) public returns (S[][1] memory) {
        s2 = _a;
        return s2;
    }

    function test3(S[1][] calldata _a) public returns (S[1][] memory) {
        s3 = _a;
        return s3;
    }
}


// ====
// compileViaYul: true
// ----
// test1((uint8[],uint8[2])[][]): 0x20, 2, 0x40, 0x0140, 1, 0x20, 0x60, 3, 7, 2, 1, 2, 2, 0x40, 0x0100, 0x60, 17, 19, 2, 11, 13, 0x60, 31, 37, 2, 23, 29 -> 0x20, 2, 0x40, 0x0140, 1, 0x20, 0x60, 3, 7, 2, 1, 2, 2, 0x40, 0x0100, 0x60, 17, 19, 2, 11, 13, 0x60, 31, 37, 2, 23, 29
// gas irOptimized: 304768
// test2((uint8[],uint8[2])[][1]): 0x20, 0x20, 1, 0x20, 0x60, 17, 19, 2, 11, 13 -> 0x20, 0x20, 1, 0x20, 0x60, 17, 19, 2, 11, 13
// gas irOptimized: 116476
// test3((uint8[],uint8[2])[1][]): 0x20, 2, 0x40, 0x0120, 0x20, 0x60, 3, 7, 2, 1, 2, 0x20, 0x60, 17, 19, 2, 11, 13 -> 0x20, 2, 0x40, 0x0120, 0x20, 0x60, 3, 7, 2, 1, 2, 0x20, 0x60, 17, 19, 2, 11, 13
// gas irOptimized: 187998
