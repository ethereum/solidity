pragma experimental SMTChecker;

contract C {
	function f(bool b) public pure {
		uint v = 1;
		if (b)
			v &= 1;
		assert(v == 1);
	}
}
// ----
// Warning: (106-112): Assertion checker does not yet implement this assignment operator.
// Warning: (116-130): Assertion violation happens here
