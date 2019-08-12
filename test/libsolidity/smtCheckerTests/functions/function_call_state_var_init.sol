pragma experimental SMTChecker;

contract C {
	uint x = f(2);

	function f(uint y) internal pure returns (uint) {
		assert(y > 1000);
		return y;
	}
}
// ----
// Warning: (116-132): Assertion violation happens here
// Warning: (116-132): Assertion violation happens here
