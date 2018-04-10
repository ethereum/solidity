contract A { constructor() public { } }
contract B is A { constructor() A public {  } }
// ----
// Warning: Modifier-style base constructor call without arguments.
