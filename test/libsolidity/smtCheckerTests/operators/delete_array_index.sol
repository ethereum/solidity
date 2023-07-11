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
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 6838: (154-155): BMC: Condition is always false.
