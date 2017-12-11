contract A { constructor() public { } }
contract B is A { constructor() A() public {  } }
// ----
// DeclarationError: Duplicated super constructor call.
