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
// DeclarationError 7576: (56-58): Undeclared identifier. Did you mean "ab"?
// DeclarationError 7576: (72-74): Undeclared identifier.
// DeclarationError 7576: (88-89): Undeclared identifier.
