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
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
