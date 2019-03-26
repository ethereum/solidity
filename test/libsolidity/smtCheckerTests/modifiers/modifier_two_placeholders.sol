pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		require(x < 10000);
		_;
		assert(x > 1);
		_;
		assert(x > 2);
		assert(x > 10);
	}

	function f() m public {
		x = x + 1;
	}
}
// ----
// Warning: (156-170): Assertion violation happens here
