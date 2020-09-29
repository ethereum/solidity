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
// Warning 1218: (163-184): CHC: Error trying to invoke SMT solver.
// Warning 1218: (188-209): CHC: Error trying to invoke SMT solver.
// Warning 4661: (188-209): BMC: Assertion violation happens here.
