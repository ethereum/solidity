pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

contract B is C {
	constructor(uint x) public {
		a = x;
	}
}

contract A is B {
	constructor(uint x) C(x + 2) B(x + 1) public {
		assert(a == x + 1);
	}
}
// ----
// Warning: (212-217): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (203-208): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (242-247): Overflow (resulting value larger than 2**256 - 1) happens here
