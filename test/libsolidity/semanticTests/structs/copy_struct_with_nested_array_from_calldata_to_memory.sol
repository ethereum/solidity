pragma abicoder v2;

contract C {
    struct S {
        uint8[1] x;
        uint8[] y;
    }

    function test(S calldata s) public returns (S memory) {
        return s;
    }
}

// ----
// test((uint8[1],uint8[])): 0x20, 3, 0x40, 2, 7, 11 -> 0x20, 3, 0x40, 2, 7, 11
// test((uint8[1],uint8[])): 0x20, 3, 0x40, 3, 17, 19, 23 -> 0x20, 3, 0x40, 3, 17, 19, 23
