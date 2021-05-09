contract C
{
	uint[] a;
	function f(bool b) public {
		a.push();
		a.push();
		a.push();
		a[2] = 3;
		require(b);
		if (b)
			delete a;
		else
			delete a[2];
		assert(a.length == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (121-122): BMC: Condition is always true.
