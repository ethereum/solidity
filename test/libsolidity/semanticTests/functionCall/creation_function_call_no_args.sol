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
// compileViaYul: also
// ----
// f() -> 2
// gas ir: 164920
// gas legacy: 119557
// gas legacyOptimized: 111385
