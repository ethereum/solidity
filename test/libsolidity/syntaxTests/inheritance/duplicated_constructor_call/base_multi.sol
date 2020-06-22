contract C { constructor(uint) public {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B { constructor() C(3) public {} }
// ----
// DeclarationError 3364: (122-126): Base constructor arguments given twice.
// DeclarationError 3364: (122-126): Base constructor arguments given twice.
