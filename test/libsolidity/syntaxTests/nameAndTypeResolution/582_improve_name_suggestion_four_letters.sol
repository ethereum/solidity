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
// DeclarationError: (52-53): Undeclared identifier.
// DeclarationError: (56-60): Undeclared identifier. Did you mean "long"?
// DeclarationError: (70-71): Undeclared identifier.
// DeclarationError: (74-78): Undeclared identifier. Did you mean "long", "log0", "log1", "log2", "log3" or "log4"?
// DeclarationError: (88-89): Undeclared identifier.
// DeclarationError: (92-96): Undeclared identifier. Did you mean "long"?
