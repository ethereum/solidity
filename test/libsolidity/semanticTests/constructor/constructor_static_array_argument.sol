contract C {
    uint256 public a;
    uint256[3] public b;

    constructor(uint256 _a, uint256[3] memory _b) {
        a = _a;
        b = _b;
    }
}

// ====
// compileViaYul: also
// ----
// constructor(): 1, 2, 3, 4 ->
// gas ir: 323680
// gas irOptimized: 203246
// gas legacy: 230282
// gas legacyOptimized: 188595
// a() -> 1
// b(uint256): 0 -> 2
// b(uint256): 1 -> 3
// b(uint256): 2 -> 4
