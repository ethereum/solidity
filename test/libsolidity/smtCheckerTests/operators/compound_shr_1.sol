pragma experimental SMTChecker;

contract C {
	function f(bool b) public pure {
		uint v = 1;
		if (b)
			v >>= 2;
		assert(v > 0);
	}
}
// ----
// Warning 9149: (106-113): Assertion checker does not yet implement this assignment operator.
// Warning 4661: (117-130): Assertion violation happens here
