contract test {
    function f() pure public {
        uint x;
        { uint x; }
        uint x;
    }
}
// ----
// Warning: (73-79): This declaration shadows an existing declaration.
// DeclarationError: (91-97): Identifier already declared.
