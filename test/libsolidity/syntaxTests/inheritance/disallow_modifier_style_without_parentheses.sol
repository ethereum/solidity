contract A { constructor() public { } }
contract B is A { constructor() A public {  } }
// ----
// Warning: (72-73): Modifier-style base constructor call without arguments.
