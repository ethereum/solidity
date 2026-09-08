contract C {
    uint256 public a;
    uint256[3] public b;

    constructor(uint256 _a, uint256[3] memory _b) {
        a = _a;
        b = _b;
    }
}
// ----
// constructor(): 1, 2, 3, 4 ->
// gas irOptimized: 148113
// gas irOptimized code: 22800
// gas legacy: 166201
// gas legacy code: 60400
// gas legacyOptimized: 149177
// gas legacyOptimized code: 26200
// gas ssaCFGOptimized: 147987
// gas ssaCFGOptimized code: 21200
// a() -> 1
// b(uint256): 0 -> 2
// b(uint256): 1 -> 3
// b(uint256): 2 -> 4
