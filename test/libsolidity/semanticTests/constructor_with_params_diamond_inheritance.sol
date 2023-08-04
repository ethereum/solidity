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
// ----
// constructor(): 2, 0 ->
// gas irOptimized: 151966
// gas legacy: 168623
// gas legacyOptimized: 144577
// i() -> 2
// j() -> 2
// k() -> 1
