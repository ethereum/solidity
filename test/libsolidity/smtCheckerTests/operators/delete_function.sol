pragma experimental SMTChecker;

contract C
{
	uint[] a;
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
		// Assertion fails as false positive because
		// setZeroValue for arrays needs \forall i . a[i] = 0
		// which is still unimplemented.
		assert(a[2] == 0);
	}
}
// ----
// Warning: (201-202): Condition is always true.
// Warning: (367-384): Assertion violation happens here
