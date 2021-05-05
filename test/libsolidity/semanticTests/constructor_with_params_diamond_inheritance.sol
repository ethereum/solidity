contract A {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) {
        i = newI;
        k = newK;
    }
}
abstract contract B is A {
    uint public j;
    constructor(uint newJ) {
        j = newJ;
    }
}
contract C is A {
    constructor(uint newI, uint newK) A(newI, newK) {}
}
contract D is B, C {
    constructor(uint newI, uint newK) B(newI) C(newI, newK + 1) {}
}
// ====
// compileViaYul: also
// ----
// constructor(): 2, 0 ->
// gas irOptimized: 160093
// gas legacy: 170665
// gas legacyOptimized: 145396
// i() -> 2
// j() -> 2
// k() -> 1
