contract C {
    function getArray() internal pure returns (uint64[40][1] storage _x) {
        assembly {
            _x.slot := sub(0, 5)
        }
    }

    function fillArray() public {
        uint64[40][1] storage _x = getArray();
        for (uint64 i = 1; i < 40; i++)
            _x[0][i] = i;
    }

    function clearArray() public {
        uint64[40][1] storage _x = getArray();
        delete _x[0];
    }

    function x() public view returns (uint64[40] memory) {
        return getArray()[0];
    }
}
// ----
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// fillArray()
// gas irOptimized: 254224
// gas legacy: 258712
// gas legacyOptimized: 257258
// gas ssaCFGOptimized: 254340
// x() -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39
// clearArray()
// gas irOptimized: 57424
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 48084
// gas legacy: 64080
// gas legacyOptimized: 56602
