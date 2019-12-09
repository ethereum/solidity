pragma experimental SMTChecker;
contract C {
	uint a;
	modifier n { _; a = 7; }
	constructor(uint x) n public {
		a = x;
	}
}

contract A is C {
	modifier m { a = 5; _; }
	constructor() C(2) public {
		assert(a == 4);
	}
}
// ----
// Warning: (202-216): Assertion violation happens here
