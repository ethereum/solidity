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
// TypeError: (106-119): Member "value" not found or not visible after argument-dependent lookup in function () returns (contract C) - did you forget the "payable" modifier?
