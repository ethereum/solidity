pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	function f(bool b) public {
		a[2][3] = 4;
		if (b)
			delete a;
		else
			delete a[2];
		assert(a[2][3] == 0);
		assert(a[1][1] == 0);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning: (174-194): Assertion violation happens here
