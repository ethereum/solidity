pragma experimental SMTChecker;
contract C {
	uint[] array;
	function f(uint x, uint p) public {
		require(x == 2);
		array[p] = 10;
		array[p] /= array[p] / x;
		assert(array[p] == x);
		assert(array[p] == 0);
	}
}
// ----
// Warning: (163-184): Error trying to invoke SMT solver.
// Warning: (188-209): Error trying to invoke SMT solver.
// Warning: (188-209): Assertion violation happens here
