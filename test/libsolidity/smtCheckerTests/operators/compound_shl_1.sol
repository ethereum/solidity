pragma experimental SMTChecker;

contract C {
	function f(bool b) public pure {
		uint v = 1000000;
		if (b)
			v <<= 2;
		assert(v > 0);
	}
}
// ----
// Warning: (112-119): Assertion checker does not yet implement this assignment operator.
// Warning: (123-136): Assertion violation happens here
