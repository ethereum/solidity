contract C {
    function f() public {
        uint x = 1 finney;
    }
}
// ----
// DeclarationError 7576: (58-64): Undeclared identifier.
