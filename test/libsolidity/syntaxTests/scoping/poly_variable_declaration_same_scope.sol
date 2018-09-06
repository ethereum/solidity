contract test {
    function f() pure public {
        uint x;
        uint x;
        uint x;
        uint x;
        uint x;
        uint x;
    }
}
// ----
// DeclarationError: (71-77): Identifier already declared.
// DeclarationError: (87-93): Identifier already declared.
// DeclarationError: (103-109): Identifier already declared.
// DeclarationError: (119-125): Identifier already declared.
// DeclarationError: (135-141): Identifier already declared.
