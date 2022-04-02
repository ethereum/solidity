contract A { constructor(uint) { } }
contract B is A(2) { constructor() A(3) {  } }
// ----
// DeclarationError 3364: (72-76='A(3)'): Base constructor arguments given twice.
