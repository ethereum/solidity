contract C {
    constructor() {}
}
contract D {
    function createC() public returns (C) {
        C c = (new C).value(2)();
        return c;
    }
}
// ----
// TypeError 8827: (107-120): Constructor for contract C must be payable for member "value" to be available.
