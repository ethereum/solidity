contract c {
    function func() public {}
    function g() public { fun(); }
}
// ----
// DeclarationError 7576: (69-72): Undeclared identifier. Did you mean "func"?
