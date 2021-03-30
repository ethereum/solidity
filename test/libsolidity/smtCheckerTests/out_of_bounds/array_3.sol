pragma experimental SMTChecker;

contract C {
	uint[] a;
	function r(uint i) public view returns (uint) {
		return a[i]; // oob access
	}
}
// ----
// Warning 6368: (115-119): CHC: Out of bounds access happens here.\nCounterexample:\na = []\ni = 0\n = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.r(0)
