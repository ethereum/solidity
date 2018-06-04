contract base {
    function f() private {}
}
contract derived is base {
    function g() public { f(); }
}
// ----
// DeclarationError: (99-100): Undeclared identifier.
