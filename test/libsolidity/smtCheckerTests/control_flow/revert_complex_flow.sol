pragma experimental SMTChecker;

contract C {
	function f(bool b, uint a) pure public {
		require(a <= 256);
		if (b)
			revert();
		uint c = a + 1;
		if (b)
			c--;
		else
			c++;
		assert(c == a);
	}
}
// ----
// Warning 6328: (183-197): CHC: Assertion violation happens here.\nCounterexample:\n\nb = false\na = 0\nc = 2\n\nTransaction trace:\nC.constructor()\nC.f(false, 0)
// Warning 6838: (155-156): BMC: Condition is always false.
