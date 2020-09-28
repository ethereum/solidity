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
// Warning 6328: (117-130): CHC: Assertion violation happens here.
