contract C {
    error e1(uint constant x);
    error e2(uint immutable x);
}
// ----
// DeclarationError 1788: (26-41): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 8297: (57-73): The "immutable" keyword can only be used for state variables.
