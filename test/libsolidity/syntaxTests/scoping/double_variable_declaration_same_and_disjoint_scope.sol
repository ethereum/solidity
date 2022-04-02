contract test {
    function f() pure public {
        uint x;
        { uint x; }
        uint x;
    }
}
// ----
// DeclarationError 2333: (91-97='uint x'): Identifier already declared.
// Warning 2519: (73-79='uint x'): This declaration shadows an existing declaration.
