contract C {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) public {
        i = newI;
        k = newK;
    }
}
contract D is C {
    constructor(uint newI, uint newK) C(newI, newK + 1) public {}
}
// ====
// compileViaYul: also
// ----
// constructor(): 2, 0 ->
// i() -> 2
// k() -> 1
