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
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
