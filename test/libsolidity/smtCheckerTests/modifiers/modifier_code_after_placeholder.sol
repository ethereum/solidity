pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
		// Fails because of overflow behavior.
		// Overflow is not reported because this assertion prevents
		// it from reaching the end of the function.
		assert(x > 1);
	}

	function f() m public {
		assert(x > 0);
		x = x + 1;
	}
}
// ----
// Warning: (245-258): Assertion violation happens here
