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
// gas irOptimized: 123966
// gas legacy: 138700
// gas legacyOptimized: 119400
// i() -> 2
// k() -> 1
