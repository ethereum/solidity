contract C {
    function f() public {
        uint x = 1 finney;
    }
}
// ----
// DeclarationError 7576: (56-64): Undeclared identifier.
