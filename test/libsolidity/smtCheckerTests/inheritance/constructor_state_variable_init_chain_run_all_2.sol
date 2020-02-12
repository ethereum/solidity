pragma experimental SMTChecker;
contract C {
	uint a;
	constructor(uint x) public {
		a = x;
	}
}

abstract contract B is C {
	uint b;
	constructor(uint x) public {
		b = x + 10;
	}
}

contract A is B {
	constructor(uint x) B(x) C(x + 2) public {
		assert(a == x + 2);
		assert(b == x + 10);
		assert(b == x + 5);
	}
}

// ----
// Warning: (171-177): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (231-236): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (171-177): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (261-266): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (283-289): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (306-311): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (294-312): Assertion violation happens here
