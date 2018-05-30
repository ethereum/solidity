contract c {
    function func() {}
    function g() public { fun(); }
}
// ----
// DeclarationError: (62-65): Undeclared identifier. Did you mean "func"?
