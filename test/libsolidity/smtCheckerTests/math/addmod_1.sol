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
// Warning 4281: (141-166): CHC: Division by zero happens here.
// Warning 6328: (170-184): CHC: Assertion violation happens here.
// Warning 4281: (263-278): CHC: Division by zero happens here.
