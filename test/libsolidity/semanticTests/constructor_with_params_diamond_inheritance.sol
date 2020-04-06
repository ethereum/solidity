contract A {
    uint public i;
    uint public k;

    constructor(uint newI, uint newK) public {
        i = newI;
        k = newK;
    }
}
abstract contract B is A {
    uint public j;
    constructor(uint newJ) public {
        j = newJ;
    }
}
contract C is A {
    constructor(uint newI, uint newK) A(newI, newK) public {}
}
contract D is B, C {
    constructor(uint newI, uint newK) B(newI) C(newI, newK + 1) public {}
}
// ====
// compileViaYul: also
// ----
// constructor(): 2, 0 ->
// i() -> 2
// j() -> 2
// k() -> 1
