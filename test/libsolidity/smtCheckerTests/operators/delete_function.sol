contract C
{
	uint[] a;
	constructor() { init(); }
	function init() internal {
		a.push();
		a.push();
		a.push();
	}
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
		init();
		assert(a[2] == 0);
		assert(a[1] == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n!(a.length <= 2)\n
// Warning 6838: (262-263): BMC: Condition is always true.
