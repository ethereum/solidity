contract C {
    function f() public {
        uint x = 1 szabo;
    }
}
// ----
// DeclarationError 7576: (58-63): Undeclared identifier.
