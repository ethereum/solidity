pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	function f(bool b) public {
		a[1][1] = 512;
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
// Warning 6328: (191-211): Assertion violation happens here
