contract C { constructor(uint) {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B { constructor() C(3) {} }
// ----
// DeclarationError 3364: (115-119): Base constructor arguments given twice.
// DeclarationError 3364: (115-119): Base constructor arguments given twice.
