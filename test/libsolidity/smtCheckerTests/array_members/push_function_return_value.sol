contract C {
	bytes x;

	function getX() internal view returns (bytes storage) {
		return x;
	}

	function s() public {
		require(x.length == 0);
		getX().push("a");
		assert(x.length == 1); // should hold but knowledge is erased due to pushing a reference
		assert(x.length == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (168-189): CHC: Assertion violation happens here.
// Warning 6328: (259-280): CHC: Assertion violation happens here.
