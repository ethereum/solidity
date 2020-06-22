contract A { constructor(uint) public { } }
contract B is A(2) { constructor() A(3) public {  } }
// ----
// DeclarationError 3364: (79-83): Base constructor arguments given twice.
