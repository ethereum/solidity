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
// gas irOptimized: 81154
// gas irOptimized code: 20000
// gas legacy: 83613
// gas legacy code: 32000
// gas ssaCFGOptimized: 81033
// gas ssaCFGOptimized code: 18600
// i() -> 2
// k() -> 0
