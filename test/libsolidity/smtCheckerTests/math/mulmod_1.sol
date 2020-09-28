pragma experimental SMTChecker;

contract C {
	function f() public pure {
		assert(mulmod(2**256 - 1, 2, 14) == 2);
		uint y = 0;
		uint x = mulmod(2**256 - 1, 10, y);
		assert(x == 1);
	}
	function g(uint x, uint y, uint k) public pure returns (uint) {
		return mulmod(x, y, k);
	}
}
// ----
// Warning 1218: (83-108): CHC: Error trying to invoke SMT solver.
// Warning 1218: (141-166): CHC: Error trying to invoke SMT solver.
// Warning 1218: (76-114): CHC: Error trying to invoke SMT solver.
// Warning 1218: (170-184): CHC: Error trying to invoke SMT solver.
// Warning 1218: (263-278): CHC: Error trying to invoke SMT solver.
// Warning 3046: (141-166): BMC: Division by zero happens here.
// Warning 3046: (263-278): BMC: Division by zero happens here.
