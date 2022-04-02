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
// DeclarationError 2333: (71-77='uint x'): Identifier already declared.
// DeclarationError 2333: (87-93='uint x'): Identifier already declared.
// DeclarationError 2333: (103-109='uint x'): Identifier already declared.
// DeclarationError 2333: (119-125='uint x'): Identifier already declared.
// DeclarationError 2333: (135-141='uint x'): Identifier already declared.
