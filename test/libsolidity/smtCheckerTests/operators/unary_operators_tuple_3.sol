pragma experimental SMTChecker;
contract C {
	function f(bool b) public pure {
		uint x;
		if (b) ++(x);
		else x += 1;
		assert(x == 1);
		assert(!b);
	}
}
// ----
// Warning 6328: (140-150): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\n\n\nTransaction trace:\nconstructor()\nf(true)
