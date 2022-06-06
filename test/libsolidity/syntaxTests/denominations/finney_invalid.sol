contract C {
    function f() public {
        uint x = 1 finney;
    }
}
// ----
// DeclarationError 7920: (58-64): Identifier not found or not unique.
