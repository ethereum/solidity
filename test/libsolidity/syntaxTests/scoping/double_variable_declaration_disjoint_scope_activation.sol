contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// DeclarationError: Identifier already declared.
