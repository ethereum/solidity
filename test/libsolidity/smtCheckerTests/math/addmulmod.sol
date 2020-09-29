pragma experimental SMTChecker;

contract C {
	function test_addmod(uint x, uint y) public pure {
		require(x % 13 == 0);
		require(y % 13 == 0);

		uint z = addmod(x, y, 13);
		assert(z == 0);
	}
	function test_mulmod(uint x, uint y) public pure {
		require(x % 13 == 0);
		require(y % 13 == 0);

		uint z = mulmod(x, y, 13);
		assert(z == 0);
	}
}
// ----
// Warning 1218: (158-174): CHC: Error trying to invoke SMT solver.
// Warning 1218: (178-192): CHC: Error trying to invoke SMT solver.
// Warning 1218: (309-325): CHC: Error trying to invoke SMT solver.
// Warning 1218: (329-343): CHC: Error trying to invoke SMT solver.
// Warning 7812: (329-343): BMC: Assertion violation might happen here.
