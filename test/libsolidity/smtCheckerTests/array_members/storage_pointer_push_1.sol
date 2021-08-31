contract C {
	int[][] array2d;
	function l() public {
		s().push();
		// False positive.
		// Knowledge is erased because `s()` is a storage pointer.
		assert(array2d[2].length > 0);
	}
	function s() internal returns (int[] storage) {
		array2d.push();
		array2d.push();
		array2d.push();
		return array2d[2];
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (159-169): CHC: Out of bounds access happens here.
// Warning 6328: (152-181): CHC: Assertion violation happens here.
