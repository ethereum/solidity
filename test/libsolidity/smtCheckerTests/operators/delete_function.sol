pragma experimental SMTChecker;

contract C
{
	uint[] a;
	function g() internal {
		delete a;
	}
	function h() internal {
		delete a[2];
	}
	function f(bool b) public {
		a[2] = 3;
		require(b);
		if (b)
			g();
		else
			h();
		assert(a[2] == 0);
		assert(a[1] == 0);
	}
}
// ----
// Warning: (201-202): Condition is always true.
