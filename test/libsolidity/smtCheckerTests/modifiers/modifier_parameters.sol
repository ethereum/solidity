pragma experimental SMTChecker;

contract C
{
	uint s;
	modifier m(uint a) {
		// Condition is always true for m(2).
		require(a > 0);
		_;
	}

	function f(uint x) m(x) m(2) m(s) public view {
		assert(x > 0);
		assert(s > 0);
	}
}
// ----
// Warning: (127-132): Condition is always true.
