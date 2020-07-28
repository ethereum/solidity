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
// Warning 6328: (123-136): Assertion violation happens here
// Warning 9149: (112-119): Assertion checker does not yet implement this assignment operator.
