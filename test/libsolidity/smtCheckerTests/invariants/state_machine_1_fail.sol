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
		// Disabled because Spacer 4.8.9 seg faults.
		//assert(x < 2);
	}
}
// ====
// SMTSolvers: z3
// ----
