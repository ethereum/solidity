pragma experimental SMTChecker;
contract C
{
	uint a;
	function f() public {
		if (a > 0)
		{
			a = a - 1;
			g();
		}
		else
			assert(a == 0);
	}
	function g() public {
		if (a > 0)
		{
			a = a - 1;
			f();
		}
		else
			assert(a == 0);
	}
}
// ----
// Warning: (206-209): Assertion checker does not support recursive function calls.
// Warning: (111-114): Assertion checker does not support recursive function calls.
