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
// ----
// Warning 6328: (152-181): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[], [], []]\n\nTransaction trace:\nC.constructor()\nState: array2d = []\nC.l()\n    C.s() -- internal call
