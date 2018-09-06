contract C { constructor(uint) public {} }
contract A is C { constructor() C(2) public {} }
contract B is C { constructor() C(2) public {} }
contract D is A, B { }
// ----
// DeclarationError: (141-163): Base constructor arguments given twice.
