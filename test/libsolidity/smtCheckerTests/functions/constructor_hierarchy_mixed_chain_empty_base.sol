pragma experimental SMTChecker;
contract F {
	uint a;
	constructor() {
		a = 2;
	}
}

contract E is F {}
contract D is E {
	constructor() {
		a = 3;
	}
}
contract C is D {}
contract B is C {
	constructor() {
		assert(a == 3);
		a = 4;
	}
}

contract A is B {
}
