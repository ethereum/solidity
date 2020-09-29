pragma experimental SMTChecker;

contract C {
	function f(uint256 d) public pure {
		uint x = addmod(1, 2, d);
		assert(x < d);
	}

	function g(uint256 d) public pure {
		uint x = mulmod(1, 2, d);
		assert(x < d);
	}

	function h() public pure returns (uint256) {
		uint x = mulmod(0, 1, 2);
		uint y = mulmod(1, 0, 2);
		assert(x == y);
		uint z = addmod(0, 1, 2);
		uint t = addmod(1, 0, 2);
		assert(z == t);
	}
}
// ----
// Warning 1218: (94-109): CHC: Error trying to invoke SMT solver.
// Warning 1218: (113-126): CHC: Error trying to invoke SMT solver.
// Warning 1218: (180-195): CHC: Error trying to invoke SMT solver.
// Warning 1218: (199-212): CHC: Error trying to invoke SMT solver.
// Warning 1218: (275-290): CHC: Error trying to invoke SMT solver.
// Warning 1218: (303-318): CHC: Error trying to invoke SMT solver.
// Warning 1218: (349-364): CHC: Error trying to invoke SMT solver.
// Warning 1218: (377-392): CHC: Error trying to invoke SMT solver.
// Warning 1218: (322-336): CHC: Error trying to invoke SMT solver.
// Warning 1218: (396-410): CHC: Error trying to invoke SMT solver.
// Warning 3046: (94-109): BMC: Division by zero happens here.
// Warning 3046: (180-195): BMC: Division by zero happens here.
