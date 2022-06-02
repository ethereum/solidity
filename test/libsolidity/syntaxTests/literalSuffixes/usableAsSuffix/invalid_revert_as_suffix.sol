contract C {
    function f() pure public {
        1 revert;
    }
}
// ----
// DeclarationError 7920: (54-60): Identifier not found or not unique.
