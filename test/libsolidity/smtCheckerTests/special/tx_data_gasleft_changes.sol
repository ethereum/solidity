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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (124-150): CHC: Assertion violation happens here.
// Warning 6328: (219-245): CHC: Assertion violation happens here.
