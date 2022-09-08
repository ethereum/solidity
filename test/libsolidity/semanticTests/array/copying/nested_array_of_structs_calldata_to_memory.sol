pragma abicoder v2;

contract C {
    struct S {
        uint8 x;
        uint8 y;
    }

    function test1(S[1][2] calldata a) public returns (S[1][2] memory) {
        return a;
    }

    function test2(S[1][] calldata a) public returns (S[1][] memory) {
        return a;
    }

    function test3(S[][2] calldata a) public returns (S[][2] memory) {
        return a;
    }
}

// ----
// test1((uint8,uint8)[1][2]): 1, 2, 3, 4 -> 1, 2, 3, 4
// test2((uint8,uint8)[1][]): 0x20, 3, 7, 11, 13, 17, 19, 23 -> 0x20, 3, 7, 11, 13, 17, 19, 23
// test3((uint8,uint8)[][2]): 0x20, 0x40, 0xa0, 1, 3, 7, 2, 11, 13, 17, 19 -> 0x20, 0x40, 0xa0, 1, 3, 7, 2, 11, 13, 17, 19
