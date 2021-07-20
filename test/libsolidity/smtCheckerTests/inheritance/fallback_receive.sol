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
		assert(x == 1);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (81-95): CHC: Assertion violation happens here.
// Warning 6328: (130-144): CHC: Assertion violation happens here.
// Warning 6328: (256-270): CHC: Assertion violation happens here.
