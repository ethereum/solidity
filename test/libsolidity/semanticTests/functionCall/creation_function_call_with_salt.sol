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
// gas irOptimized: 139112
// gas irOptimized code: 53800
// gas legacy: 145935
// gas legacy code: 95600
// gas legacyOptimized: 138529
// gas legacyOptimized code: 54600
// f() -> 2
