pragma experimental SMTChecker;
contract C {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract B is C {
}

contract B2 is C {
	constructor() public {
		assert(a == 2);
	}
}

contract A is B, B2 {
}
