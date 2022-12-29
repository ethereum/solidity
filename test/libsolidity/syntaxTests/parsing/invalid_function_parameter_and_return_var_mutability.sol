contract test {
    function f1(uint immutable a) public returns (uint immutable) { }
    function f2(uint constant a) public returns (uint constant) { }
}
// ----
// DeclarationError 8297: (32-48): The "immutable" keyword can only be used for state variables.
// DeclarationError 8297: (66-80): The "immutable" keyword can only be used for state variables.
// DeclarationError 1788: (102-117): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 1788: (135-148): The "constant" keyword can only be used for state variables or variables at file level.
