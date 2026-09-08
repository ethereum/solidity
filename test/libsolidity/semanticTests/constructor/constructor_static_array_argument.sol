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
// gas irOptimized: 148129
// gas irOptimized code: 23000
// gas legacy: 166201
// gas legacy code: 60400
// gas legacyOptimized: 149177
// gas legacyOptimized code: 26200
// gas ssaCFGOptimized: 147985
// gas ssaCFGOptimized code: 21400
// a() -> 1
// b(uint256): 0 -> 2
// b(uint256): 1 -> 3
// b(uint256): 2 -> 4
