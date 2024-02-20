contract C {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) {
        i = newI;
        k = newK;
    }
}
// ----
// constructor(): 2, 0 ->
// gas irOptimized: 101370
// gas legacy: 115614
// i() -> 2
// k() -> 0
