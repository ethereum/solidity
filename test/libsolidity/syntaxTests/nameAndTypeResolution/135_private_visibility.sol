contract base {
    function f() private {}
}
contract derived is base {
    function g() public { f(); }
}
// ----
// DeclarationError 7576: (99-100): Undeclared identifier.
