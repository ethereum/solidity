pragma experimental SMTChecker;

contract c {
	mapping(uint => uint) x;
	mapping(uint => uint) y;
	function f(bool cond) public {
		mapping(uint => uint) storage a = cond ? x : y;
		x[2] = 1;
		y[2] = 2;
		a[2] = 3;
		// False positive since aliasing is not yet supported.
		if (cond)
			assert(a[2] == x[2] && a[2] != y[2]);
		else
			assert(a[2] == y[2] && a[2] != x[2]);
	}
}
// ----
// Warning: (166-178): Internal error: Expression undefined for SMT solver.
// Warning: (288-324): Assertion violation happens here
// Warning: (336-372): Assertion violation happens here
