pragma experimental SMTChecker;
contract C {
	int[][] array2d;
	function l() public {
		array2d.push().push();
		assert(array2d.length > 2);
		assert(array2d[array2d.length - 1].length > 3);
	}
}
// ----
// Warning 6328: (113-139): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[0]]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
// Warning 6328: (143-189): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[0]]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
