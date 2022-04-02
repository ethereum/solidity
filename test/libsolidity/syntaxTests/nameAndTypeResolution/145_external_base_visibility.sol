contract base {
    function f() external {}
}
contract derived is base {
    function g() public { base.f(); }
}
// ----
// TypeError 3419: (100-108='base.f()'): Cannot call function via contract type name.
