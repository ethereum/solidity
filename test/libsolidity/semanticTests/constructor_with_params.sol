contract C {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) {
        i = newI;
        k = newK;
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 2, 0 ->
// gas ir: 198085
// gas irOptimized: 137306
// gas legacy: 133000
// gas legacyOptimized: 118666
// i() -> 2
// k() -> 0
