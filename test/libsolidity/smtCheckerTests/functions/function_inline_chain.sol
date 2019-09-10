pragma experimental SMTChecker;

contract C
{
	uint x;
	uint y;
	uint z;

	function f() public {
		if (x == 1)
			x = 2;
		else
			x = 1;
		g();
		assert(y == 1);
	}

	function g() public {
		y = 1;
		h();
		assert(z == 1);
	}

	function h() public {
		z = 1;
		x = 1;
		f();
		// This fails for the following calls to the contract:
		// h()
		// g() h()
		// It does not fail for f() g() h() because in that case
		// h() will not inline f() since it already is in the callstack.
		assert(x == 1);
	}
}
// ----
// Warning: (271-274): Assertion checker does not support recursive function calls.
// Warning: (140-143): Assertion checker does not support recursive function calls.
// Warning: (483-497): Assertion violation happens here
// Warning: (201-204): Assertion checker does not support recursive function calls.
// Warning: (483-497): Assertion violation happens here
