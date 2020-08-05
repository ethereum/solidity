contract C {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) {
        i = newI;
        k = newK;
    }
}
contract D is C(2, 1) {}
// ====
// compileViaYul: also
// ----
// i() -> 2
// k() -> 1
