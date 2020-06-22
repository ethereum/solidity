pragma experimental SMTChecker;

contract C {
	uint x = 2;
	constructor () public {
		assert(x == 2);
		assert(x == 3);
	}
}
// ----
// Warning 4661: (104-118): Assertion violation happens here
