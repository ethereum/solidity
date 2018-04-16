contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// DeclarationError: (75-81): Identifier already declared.
