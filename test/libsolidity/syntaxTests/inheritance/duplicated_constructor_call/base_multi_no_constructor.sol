contract C { constructor(uint) public {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B {}
// ----
// DeclarationError: (87-108): Base constructor arguments given twice.
