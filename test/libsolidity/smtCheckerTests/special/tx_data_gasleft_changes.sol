pragma experimental SMTChecker;

contract C {
	uint gleft;

	function f() public payable {
		gleft = gasleft();

		fi();

		assert(gleft == gasleft());
		assert(gleft >= gasleft());
	}

	function fi() internal view {
		assert(gleft == gasleft());
	}
}
// ----
// Warning 6328: (124-150): CHC: Assertion violation happens here.\nCounterexample:\ngleft = 1\n\n\n\nTransaction trace:\nconstructor()\nState: gleft = 0\nf()
// Warning 6328: (219-245): CHC: Assertion violation happens here.\nCounterexample:\ngleft = 1\n\n\n\nTransaction trace:\nconstructor()\nState: gleft = 0\nf()
