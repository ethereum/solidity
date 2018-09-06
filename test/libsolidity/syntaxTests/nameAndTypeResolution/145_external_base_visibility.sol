contract base {
    function f() external {}
}
contract derived is base {
    function g() public { base.f(); }
}
// ----
// TypeError: (100-106): Member "f" not found or not visible after argument-dependent lookup in type(contract base).
