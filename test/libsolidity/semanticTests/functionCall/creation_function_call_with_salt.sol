contract C {
    uint public i;
    constructor(uint newI) {
        i = newI;
    }
}
contract D {
    C c;
    constructor(uint v) {
        c = new C{salt: "abc"}(v);
    }
    function f() public returns (uint r) {
        return c.i();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// constructor(): 2 ->
// gas irOptimized: 192826
// gas legacy: 241536
// gas legacyOptimized: 193129
// f() -> 2
