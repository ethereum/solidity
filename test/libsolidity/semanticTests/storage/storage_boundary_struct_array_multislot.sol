pragma abicoder v2;

contract C {
    struct S {
        uint256 a;
        uint256 b;
        uint256 c;
        // 3 slots per struct
    }

    struct Canary {
        uint256 value;
    }

    function getBoundaryArray() internal pure returns (S[10][1] storage arr) {
        // 10 structs, each 3 slots = 30 slots total
        assembly {
            arr.slot := sub(0, 15)
        }
    }

    function getDest() internal pure returns (S[10][1] storage arr) {
        assembly {
            arr.slot := 16
        }
    }

    function getCanary() internal pure returns (Canary storage canary) {
        assembly {
            canary.slot := 15
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
                a: 1 + i * 3,
                b: 2 + i * 3,
                c: 3 + i * 3
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
                a: 31 + i * 3,
                b: 32 + i * 3,
                c: 33 + i * 3
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
// boundaryArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// destArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// fillBoundaryArray()
// gas irOptimized: 688716
// gas legacy: 700075
// gas legacyOptimized: 691605
// gas ssaCFGOptimized: 688651
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
// destArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// copyFromBoundary()
// gas irOptimized: 748776
// gas legacy: 767297
// gas legacyOptimized: 748756
// gas ssaCFGOptimized: 748795
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
// destArray() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
// fillDestArray()
// gas irOptimized: 175620
// gas legacy: 187093
// gas legacyOptimized: 178625
// gas ssaCFGOptimized: 175555
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
// destArray() -> 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60
// copyToBoundary()
// gas irOptimized: 235820
// gas legacy: 254252
// gas legacyOptimized: 235722
// gas ssaCFGOptimized: 235839
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60
// destArray() -> 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60
// deleteBoundaryArray()
// gas irOptimized: 137821
// gas legacy: 137971
// gas legacyOptimized: 137750
// gas ssaCFGOptimized: 137816
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// boundaryArray() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// destArray() -> 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60
