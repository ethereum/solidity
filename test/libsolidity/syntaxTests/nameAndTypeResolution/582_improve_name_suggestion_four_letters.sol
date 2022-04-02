contract c {
    function f () public
    {
        a = land;
        a = lost;
        a = lang;
    }
    uint256 long;
    uint256 abc;
}
// ----
// DeclarationError 7576: (52-53='a'): Undeclared identifier.
// DeclarationError 7576: (56-60='land'): Undeclared identifier. Did you mean "long"?
// DeclarationError 7576: (70-71='a'): Undeclared identifier.
// DeclarationError 7576: (74-78='lost'): Undeclared identifier. Did you mean "long"?
// DeclarationError 7576: (88-89='a'): Undeclared identifier.
// DeclarationError 7576: (92-96='lang'): Undeclared identifier. Did you mean "long"?
