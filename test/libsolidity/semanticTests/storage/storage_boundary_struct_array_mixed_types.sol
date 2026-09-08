pragma abicoder v2;

contract C {
    struct S {
        uint256 a;      // slot 0 (bytes 0-31)
        uint128 b;      // slot 1 (bytes 0-15)
        uint64 c;       // slot 1 (bytes 16-23)
        bytes32 d;      // slot 2 (bytes 0-31)
        bool e;         // slot 3 (byte 0)
        // Total: 4 slots per struct
    }

    struct Canary {
        uint256 value;
    }

    function getBoundaryArray() internal pure returns (S[10][1] storage arr) {
        // 10 structs * 4 slots = 40 slots total
        // Starts at -20, ends at slot 19
        assembly {
            arr.slot := sub(0, 20)
        }
    }

    function getDest() internal pure returns (S[10][1] storage arr) {
        assembly {
            arr.slot := 21
        }
    }

    function getCanary() internal pure returns (Canary storage canary) {
        // Array ends at slot 19, canary at slot 20
        assembly {
            canary.slot := 20
        }
    }

    constructor() {
        Canary storage canary = getCanary();
        canary.value = type(uint256).max;
    }

    function fillBoundaryArray() public {
        S[10][1] storage arr = getBoundaryArray();
        for (uint i = 0; i < 10; i++) {
            arr[0][i] = S({
                a: 1 + i * 5,
                b: uint128(2 + i * 5),
                c: uint64(3 + i * 5),
                d: bytes32(uint256(4 + i * 5)),
                e: true
            });
        }
    }

    function deleteBoundaryArray() public {
        S[10][1] storage arr = getBoundaryArray();
        delete arr[0];
    }

    function copyFromBoundary() public {
        S[10][1] storage source = getBoundaryArray();
        S[10][1] storage dest = getDest();
        dest[0] = source[0];
    }

    function copyToBoundary() public {
        S[10][1] storage source = getDest();
        S[10][1] storage dest = getBoundaryArray();
        dest[0] = source[0];
    }

    function fillDestArray() public {
        S[10][1] storage dest = getDest();
        for (uint i = 0; i < 10; i++) {
            dest[0][i] = S({
                a: 51 + i * 5,
                b: uint128(52 + i * 5),
                c: uint64(53 + i * 5),
                d: bytes32(uint256(54 + i * 5)),
                e: true
            });
        }
    }

    function boundaryArray() public view returns (S[10] memory) {
        return getBoundaryArray()[0];
    }

    function destArray() public view returns (S[10] memory) {
        return getDest()[0];
    }

    function canaryValue() public view returns (uint256) {
        return getCanary().value;
    }
}
// ----
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
// fillBoundaryArray()
// gas irOptimized: 912522
// gas legacy: 930728
// gas legacyOptimized: 916628
// gas ssaCFGOptimized: 915267
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, true, 6, 7, 8, 9, true, 11, 12, 13, 14, true, 16, 17, 18, 19, true, 21, 22, 23, 24, true, 26, 27, 28, 29, true, 31, 32, 33, 34, true, 36, 37, 38, 39, true, 41, 42, 43, 44, true, 46, 47, 48, 49, true
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
// copyFromBoundary()
// gas irOptimized: 994579
// gas legacy: 1023407
// gas legacyOptimized: 994746
// gas ssaCFGOptimized: 996288
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, true, 6, 7, 8, 9, true, 11, 12, 13, 14, true, 16, 17, 18, 19, true, 21, 22, 23, 24, true, 26, 27, 28, 29, true, 31, 32, 33, 34, true, 36, 37, 38, 39, true, 41, 42, 43, 44, true, 46, 47, 48, 49, true
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 1, 2, 3, 4, true, 6, 7, 8, 9, true, 11, 12, 13, 14, true, 16, 17, 18, 19, true, 21, 22, 23, 24, true, 26, 27, 28, 29, true, 31, 32, 33, 34, true, 36, 37, 38, 39, true, 41, 42, 43, 44, true, 46, 47, 48, 49, true
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
// fillDestArray()
// gas irOptimized: 200426
// gas legacy: 218746
// gas legacyOptimized: 204648
// gas ssaCFGOptimized: 203171
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, true, 6, 7, 8, 9, true, 11, 12, 13, 14, true, 16, 17, 18, 19, true, 21, 22, 23, 24, true, 26, 27, 28, 29, true, 31, 32, 33, 34, true, 36, 37, 38, 39, true, 41, 42, 43, 44, true, 46, 47, 48, 49, true
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 51, 52, 53, 54, true, 56, 57, 58, 59, true, 61, 62, 63, 64, true, 66, 67, 68, 69, true, 71, 72, 73, 74, true, 76, 77, 78, 79, true, 81, 82, 83, 84, true, 86, 87, 88, 89, true, 91, 92, 93, 94, true, 96, 97, 98, 99, true
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
// copyToBoundary()
// gas irOptimized: 282623
// gas legacy: 311362
// gas legacyOptimized: 282712
// gas ssaCFGOptimized: 284332
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 51, 52, 53, 54, true, 56, 57, 58, 59, true, 61, 62, 63, 64, true, 66, 67, 68, 69, true, 71, 72, 73, 74, true, 76, 77, 78, 79, true, 81, 82, 83, 84, true, 86, 87, 88, 89, true, 91, 92, 93, 94, true, 96, 97, 98, 99, true
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 51, 52, 53, 54, true, 56, 57, 58, 59, true, 61, 62, 63, 64, true, 66, 67, 68, 69, true, 71, 72, 73, 74, true, 76, 77, 78, 79, true, 81, 82, 83, 84, true, 86, 87, 88, 89, true, 91, 92, 93, 94, true, 96, 97, 98, 99, true
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
// deleteBoundaryArray()
// gas irOptimized: 177968
// gas legacy: 180995
// gas legacyOptimized: 178182
// gas ssaCFGOptimized: 177963
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 113169
// gas legacy: 120742
// gas legacyOptimized: 112518
// gas ssaCFGOptimized: 113258
// destArray() -> 51, 52, 53, 54, true, 56, 57, 58, 59, true, 61, 62, 63, 64, true, 66, 67, 68, 69, true, 71, 72, 73, 74, true, 76, 77, 78, 79, true, 81, 82, 83, 84, true, 86, 87, 88, 89, true, 91, 92, 93, 94, true, 96, 97, 98, 99, true
// gas irOptimized: 113078
// gas legacy: 120738
// gas legacyOptimized: 112505
// gas ssaCFGOptimized: 113167
