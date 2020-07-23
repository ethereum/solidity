contract C { constructor(uint) {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B {}
// ----
// DeclarationError 3364: (80-101): Base constructor arguments given twice.
