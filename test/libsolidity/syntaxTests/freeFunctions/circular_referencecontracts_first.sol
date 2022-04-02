// Checks that error is triggered no matter which order
contract D {
    function f() public {
        l();
    }
}
contract C {
    constructor() { new D(); }
}
function l() {
    s();
}
function s() {
    new C();
}
// ----
// TypeError 7813: (207-212='new C'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (149-154='new D'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
