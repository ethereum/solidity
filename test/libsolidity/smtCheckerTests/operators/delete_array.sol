pragma experimental SMTChecker;

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
// ----
// Warning 6838: (154-155): BMC: Condition is always true.
