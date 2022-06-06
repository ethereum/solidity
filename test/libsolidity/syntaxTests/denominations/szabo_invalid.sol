contract C {
    function f() public {
        uint x = 1 szabo;
    }
}
// ----
// DeclarationError 7920: (58-63): Identifier not found or not unique.
