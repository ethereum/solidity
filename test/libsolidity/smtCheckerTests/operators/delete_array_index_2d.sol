pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	function f(bool b) public {
		require(a[2][3] == 4);
		if (b)
			delete a;
		else
			delete a[2];
		// Fails as false positive since
		// setZeroValue for arrays needs forall
		// which is unimplemented.
		assert(a[2][3] == 0);
	}
}
// ----
// Warning: (266-286): Assertion violation happens here
