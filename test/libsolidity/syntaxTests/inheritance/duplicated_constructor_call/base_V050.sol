pragma experimental "v0.5.0";

contract A { constructor(uint) public { } }
contract B is A(2) { constructor() A(3) public {  } }
// ----
// DeclarationError: Base constructor arguments given twice.
