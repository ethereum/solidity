pragma experimental SMTChecker;

// 2 warnings, receive and A.g
contract A {
	uint x;

	receive () external payable {
		assert(x == 1);
	}
	function g() public view {
		assert(x == 1);
	}
}

// 3 warnings, fallback, A.receive and A.g
contract B is A {
	uint y;

	fallback () external {
		assert(x == 1);
	}
}
// ----
// Warning 6328: (120-134): Assertion violation happens here
// Warning 6328: (169-183): Assertion violation happens here
// Warning 6328: (288-302): Assertion violation happens here
