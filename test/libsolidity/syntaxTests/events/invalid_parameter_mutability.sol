contract test {
    event e1(uint constant a);
    event e2(uint immutable a);
}
// ----
// DeclarationError 1788: (29-44): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 8297: (60-76): The "immutable" keyword can only be used for state variables.
