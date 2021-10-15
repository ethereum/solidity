contract C {
	function r(bytes4 x, uint y) public pure returns (bytes1) {
		return x[y]; // oob access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (83-87): CHC: Out of bounds access happens here.\nCounterexample:\n\nx = 0x0\ny = 4\n = 0x0\n\nTransaction trace:\nC.constructor()\nC.r(0x0, 4)
