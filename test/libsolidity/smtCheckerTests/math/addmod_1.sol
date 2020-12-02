pragma experimental SMTChecker;

contract C {
	function f() public pure {
		assert(addmod(2**256 - 1, 10, 9) == 7);
		uint y = 0;
		uint x = addmod(2**256 - 1, 10, y);
		assert(x == 1);
	}
	function g(uint x, uint y, uint k) public pure returns (uint) {
		return addmod(x, y, k);
	}
}
// ----
// Warning 4281: (141-166): CHC: Division by zero happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (170-184): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 4281: (263-278): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\nk = 0\n = 0\n\nTransaction trace:\nconstructor()\ng(0, 0, 0)
