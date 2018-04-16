contract test {
    function f() pure public {
        { uint x; }
        { uint x; }
    }
}
// ----
// DeclarationError: (77-83): Identifier already declared.
