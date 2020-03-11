pragma experimental SMTChecker;

contract C
{
	uint y;

	function f() public {
		if (y != 1)
			g();
		assert(y == 1);
	}

	function g() internal {
		y = 1;
		h();
	}

	function h() internal {
		f();
		assert(y == 1);
	}
}
// ----
