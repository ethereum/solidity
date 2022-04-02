contract test {
    function f() pure public {
        uint x;
        uint x;
    }
}
// ----
// DeclarationError 2333: (71-77='uint x'): Identifier already declared.
