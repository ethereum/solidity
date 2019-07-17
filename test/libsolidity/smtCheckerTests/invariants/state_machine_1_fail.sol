pragma experimental SMTChecker;

contract C {
	uint x;

	function f() public {
		if (x == 0)
			x = 1;
	}

	function g() public {
		if (x == 1)
			x = 2;
	}

	function h() public {
		if (x == 2)
			x = 0;
	}

	function j() public {
		if (x < 2)
			x = 100;
	}

	// Fails due to j.
	function i() public view {
		assert(x < 2);
	}
}
// ----
// Warning: (311-324): Assertion violation happens here
