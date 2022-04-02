contract A { constructor(uint) { } }
contract B is A(2) { constructor() {  } }
contract C is B { constructor() A(3) {  } }
// ----
// DeclarationError 3364: (111-115='A(3)'): Base constructor arguments given twice.
