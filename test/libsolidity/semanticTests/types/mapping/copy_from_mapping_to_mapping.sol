pragma abicoder v2;

contract C {
    struct S {
        uint8[3] x;
        uint8[][] y;
        uint16 z;
    }

    mapping (uint8 => S) src;
    mapping (uint8 => S) dst;

    constructor() {
        uint8[] memory d = new uint8[](2);
        d[0] = 3;
        d[1] = 4;

        uint8[][] memory y = new uint8[][](2);
        y[0] = d;
        y[1] = d;

        src[0] = S({x: [7, 8, 9], y: y, z: 13});
    }

    function f() public returns (S memory) {
        dst[0] = src[0];
        return dst[0];
    }
}

// ----
// f() -> 0x20, 7, 8, 9, 0xa0, 13, 2, 0x40, 0xa0, 2, 3, 4, 2, 3, 4
// gas irOptimized: 197113
// gas legacy: 199986
// gas legacyOptimized: 196847
