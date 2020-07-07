contract C {
    constructor() payable {}
}
contract D {
    function createC() public returns (C) {
        C c = (new C).value(2)();
        return c;
    }
}
// ----
// TypeError 1621: (115-128): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
