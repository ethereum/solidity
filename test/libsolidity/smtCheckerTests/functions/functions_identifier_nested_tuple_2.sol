pragma experimental SMTChecker;

library L {
	struct S {
		uint256[] data;
	}
	function f(S memory _s) internal pure returns (uint256) {
		require(_s.data.length > 0);
		return 42;
	}
}

contract C {
	using L for L.S;
	function f() public pure returns (uint256 y) {
		L.S memory x;
		y = (x.f)();
		assert(y == 42); // should hold
	}
}
// ----
// Warning 6031: (289-292): Internal error: Expression undefined for SMT solver.
// Warning 6031: (289-292): Internal error: Expression undefined for SMT solver.
