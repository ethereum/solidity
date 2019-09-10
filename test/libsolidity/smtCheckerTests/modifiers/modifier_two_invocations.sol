pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		// Condition is always true for the second invocation.
		require(x > 0);
		require(x < 10000);
		_;
		assert(x > 1);
	}

	function f() m m public {
		x = x + 1;
	}
}
// ----
