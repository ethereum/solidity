contract C {
    struct S {
        uint64 a;
        uint64 b;
        uint64 c;
        uint64 d;
        // All fit in one slot (4 * 64 = 256 bits)
    }

    S[5] source;
    S[10] dest;
    uint256 public canary = type(uint256).max;

    function fillSource() public {
        for (uint i = 0; i < 5; i++) {
            source[i] = S({
                a: uint64(1 + i * 4),
                b: uint64(2 + i * 4),
                c: uint64(3 + i * 4),
                d: uint64(4 + i * 4)
            });
        }
    }

    function fillDest() public {
        for (uint i = 0; i < 10; i++) {
            dest[i] = S({
                a: uint64(100 + i * 4),
                b: uint64(101 + i * 4),
                c: uint64(102 + i * 4),
                d: uint64(103 + i * 4)
            });
        }
    }

    function copySourceToDest() public {
        dest = source;
    }

    function deleteSource() public {
        delete source;
    }

    function deleteDest() public {
        delete dest;
    }

    function getSourceAsUint() public view returns (uint64[20] memory result) {
        for (uint i = 0; i < 5; i++) {
            result[i * 4] = source[i].a;
            result[i * 4 + 1] = source[i].b;
            result[i * 4 + 2] = source[i].c;
            result[i * 4 + 3] = source[i].d;
        }
    }

    function getDestAsUint() public view returns (uint64[40] memory result) {
        for (uint i = 0; i < 10; i++) {
            result[i * 4] = dest[i].a;
            result[i * 4 + 1] = dest[i].b;
            result[i * 4 + 2] = dest[i].c;
            result[i * 4 + 3] = dest[i].d;
        }
    }
}
// ----
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// getDestAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// fillSource()
// gas irOptimized: 135015
// gas legacy: 146851
// gas legacyOptimized: 137549
// gas ssaCFGOptimized: 137975
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
// fillDest()
// gas irOptimized: 248703
// gas legacy: 272468
// gas legacyOptimized: 253871
// gas ssaCFGOptimized: 254263
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
// getDestAsUint() -> 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139
// copySourceToDest()
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
// getDestAsUint() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// deleteSource()
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// getDestAsUint() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// deleteDest()
// canary() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getSourceAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// getDestAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
