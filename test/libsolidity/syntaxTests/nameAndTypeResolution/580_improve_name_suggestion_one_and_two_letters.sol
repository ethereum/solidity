contract c {
    function f () public
    {
        a = ac;
        a = cd;
        a = b;
    }
    uint256 a;
    uint256 ab;
}
// ----
// DeclarationError: (56-58): Undeclared identifier. Did you mean "ab"?
// DeclarationError: (72-74): Undeclared identifier.
// DeclarationError: (88-89): Undeclared identifier.
