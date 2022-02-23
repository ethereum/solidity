contract test {
    function f() pure public {
        uint x;
        { uint x; }
        uint x;
    }
}
// ----
// DeclarationError 2333: (91-97): Identifier already declared.
// Warning 2519: (73-79): This declaration shadows an existing declaration.
