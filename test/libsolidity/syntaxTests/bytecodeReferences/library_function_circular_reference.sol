library L {
    function f() internal {
        new C();
    }
}

contract D {
    function f() public {
        L.f();
    }
}
contract C {
    constructor() { new D(); }
}

// ----
// TypeError 7813: (48-53='new C'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (161-166='new D'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
