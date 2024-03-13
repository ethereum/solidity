contract C {
    uint256 constant LEN = 3;
    uint256[LEN] public a;

    constructor(uint256[LEN] memory _a) {
        a = _a;
    }
}
// ----
// constructor(): 1, 2, 3 ->
// gas irOptimized: 124991
// gas irOptimized code: 14800
// gas legacy: 134317
// gas legacy code: 46200
// gas legacyOptimized: 127166
// gas legacyOptimized code: 23400
// a(uint256): 0 -> 1
// a(uint256): 1 -> 2
// a(uint256): 2 -> 3
