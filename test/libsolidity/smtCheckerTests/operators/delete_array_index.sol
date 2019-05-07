pragma experimental SMTChecker;

contract C
{
	uint[] a;
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
// ----
// Warning: (119-120): Condition is always false.
