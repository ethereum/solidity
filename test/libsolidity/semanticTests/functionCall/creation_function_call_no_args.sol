contract C {
    uint public i;
    constructor() {
        i = 2;
    }
}
contract D {
    function f() public returns (uint r) {
        return new C().i();
    }
}
// ====
// targetContract: D
// ----
// f() -> 2
// gas legacy: 76585
// gas legacy code: 23600
