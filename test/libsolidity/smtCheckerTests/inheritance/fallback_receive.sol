pragma experimental SMTChecker;

// 2 warnings, fallback and A.g
contract A {
	uint x;

	fallback () external {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 3 warnings, receive, A.fallback and A.g
contract B is A {
	uint y;

	receive () external payable {
		assert(x == 0);
	}
}
// ----
// Warning: (114-128): Assertion violation happens here
// Warning: (163-177): Assertion violation happens here
// Warning: (289-303): Assertion violation happens here
// Warning: (114-128): Assertion violation happens here
// Warning: (163-177): Assertion violation happens here
