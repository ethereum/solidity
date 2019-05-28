pragma experimental SMTChecker;

contract C
{
	uint[] a;
	function f(bool b) public {
		a[2] = 3;
		require(b);
		if (b)
			delete a;
		else
			delete a[2];
		// Assertion fails as false positive because
		// setZeroValue for arrays needs \forall i . a[i] = 0
		// which is still unimplemented.
		assert(a[2] == 0);
	}
}
// ----
// Warning: (118-119): Condition is always true.
// Warning: (297-314): Assertion violation happens here
