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
		assert(a[2][3] == 0);
		assert(a[1][1] == 0);
	}
}
// ----
// Warning: (184-204): Assertion violation happens here
