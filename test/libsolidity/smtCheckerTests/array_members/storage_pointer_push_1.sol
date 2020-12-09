pragma experimental SMTChecker;
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
// ----
// Warning 6328: (184-213): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[], [], []]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
