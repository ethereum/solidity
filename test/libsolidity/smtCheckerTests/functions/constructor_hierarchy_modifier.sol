pragma experimental SMTChecker;
contract C {
	uint a;
	modifier n { _; a = 7; }
	constructor(uint x) n {
		a = x;
	}
}

contract A is C {
	modifier m { a = 5; _; }
	constructor() C(2) {
		assert(a == 4);
	}
}
// ----
// Warning 4661: (188-202): Assertion violation happens here
