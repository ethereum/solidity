contract base {
    function f() private {}
}
contract derived is base {
    function g() public { base.f(); }
}
// ----
// TypeError: (99-105): Member "f" not found or not visible after argument-dependent lookup in type(contract base)
