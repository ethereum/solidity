pragma experimental SMTChecker;

// 2 warnings, receive and A.g
contract A {
	uint x;

	receive () external virtual payable {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 2 warnings, receive and A.g
contract B is A {
	uint y;

	receive () external payable override {
		assert(x == 0);
	}
}
// ----
// Warning: (128-142): Assertion violation happens here
// Warning: (177-191): Assertion violation happens here
// Warning: (300-314): Assertion violation happens here
// Warning: (177-191): Assertion violation happens here
