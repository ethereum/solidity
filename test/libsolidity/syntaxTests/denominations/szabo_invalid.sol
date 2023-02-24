contract C {
    function f() public {
        uint x = 1 szabo;
    }
}
// ----
// DeclarationError 7576: (56-63): Undeclared identifier.
