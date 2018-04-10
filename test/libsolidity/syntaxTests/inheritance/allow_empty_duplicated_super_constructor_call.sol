contract A { constructor() public { } }
contract B1 is A { constructor() A() public {  } }
contract B2 is A { constructor() A public {  } }
