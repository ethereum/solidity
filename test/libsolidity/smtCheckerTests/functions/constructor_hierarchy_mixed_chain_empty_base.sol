pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() public {
		a = 2;
	}
}

contract E is F {}
contract D is E {
	constructor() public {
		a = 3;
	}
}
contract C is D {}
contract B is C {
	constructor() public {
		assert(a == 3);
		a = 4;
	}
}

contract A is B {
}
