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
// Warning 6328: (122-148): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[[0]]]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
// Warning 6328: (202-218): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[[0]]]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
// Warning 6328: (222-278): CHC: Assertion violation happens here.\nCounterexample:\narray2d = [[[0]]]\n\n\n\nTransaction trace:\nconstructor()\nState: array2d = []\nl()
