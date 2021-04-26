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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (91-117): CHC: Assertion violation happens here.
// Warning 6328: (186-212): CHC: Assertion violation happens here.
