pragma experimental SMTChecker;

contract C
{
	modifier m {
		uint x = 2;
		_;
	}

	function f(uint x) m public pure {
		assert(x == 2);
	}
}
// ----
// Warning 6328: (121-135): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
