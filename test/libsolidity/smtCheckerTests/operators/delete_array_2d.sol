pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	function f() public {
		require(a[2][3] == 4);
		delete a;
		// Fails as false positive.
		// setZeroValue needs forall for arrays.
		assert(a[2][3] == 0);
	}
}
// ----
// Warning: (194-214): Assertion violation happens here
