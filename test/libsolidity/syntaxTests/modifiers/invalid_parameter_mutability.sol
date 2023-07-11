contract A {
    modifier mod1(uint constant a) { _; }
    modifier mod2(uint immutable a) { _; }
}
// ----
// DeclarationError 1788: (31-46): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 8297: (73-89): The "immutable" keyword can only be used for state variables.