pragma experimental SMTChecker;
contract C {
	int[][][] array2d;
	function l() public {
		array2d.push().push().push();
		assert(array2d.length > 2);
		uint last = array2d[array2d.length - 1].length;
		assert(last > 3);
		assert(array2d[array2d.length - 1][last - 1].length > 4);
	}
}
// ----
// Warning 4661: (122-148): Assertion violation happens here
// Warning 4661: (202-218): Assertion violation happens here
// Warning 4661: (222-278): Assertion violation happens here
