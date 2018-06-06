contract c {
    function f() external {}
    function g() public { f(); }
}
// ----
// DeclarationError: (68-69): Undeclared identifier. Did you mean "f"?
