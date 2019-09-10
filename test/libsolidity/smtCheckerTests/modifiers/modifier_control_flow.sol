pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		if (x == 0)
			_;
	}

	function f() m public view {
		assert(x == 0);
		assert(x > 1);
	}
}
// ----
// Warning: (144-157): Assertion violation happens here
