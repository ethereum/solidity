contract C {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) {
        i = newI;
        k = newK;
    }
}
contract D is C {
    constructor(uint newI, uint newK) C(newI, newK + 1) {}
}
// ----
// constructor(): 2, 0 ->
// gas irOptimized: 101581
// gas irOptimized code: 20200
// gas legacy: 105192
// gas legacy code: 32000
// gas legacyOptimized: 101503
// gas legacyOptimized code: 17000
// i() -> 2
// k() -> 1
