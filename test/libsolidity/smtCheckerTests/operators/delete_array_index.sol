contract C
{
	uint[] a;
	constructor() {
		a.push();
		a.push();
		a.push();
		a.push();
	}
	function f(bool b) public {
		a[2] = 3;
		require(!b);
		if (b)
			delete a;
		else
			delete a[2];
		assert(a[2] == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n!(a.length <= 2)\n
// Warning 6838: (154-155): BMC: Condition is always false.
