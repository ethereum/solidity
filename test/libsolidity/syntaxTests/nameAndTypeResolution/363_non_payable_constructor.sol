contract C {
    constructor() public { }
}
contract D {
    function f() public returns (uint) {
        (new C).value(2)();
        return 2;
    }
}
// ----
// TypeError: (106-119): Constructor for contract C must be payable for member "value" to be available.
