contract C {
    function C() { }
}
contract D {
    function f() public returns (uint) {
        (new C).value(2)();
        return 2;
    }
}
// ----
// Warning: (17-33): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (98-111): Member "value" not found or not visible after argument-dependent lookup in function () returns (contract C) - did you forget the "payable" modifier?
