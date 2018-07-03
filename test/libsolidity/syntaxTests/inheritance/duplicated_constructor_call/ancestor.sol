contract A { constructor(uint) public { } }
contract B is A(2) { constructor() public {  } }
contract C is B { constructor() A(3) public {  } }
// ----
// DeclarationError: (125-129): Base constructor arguments given twice.
