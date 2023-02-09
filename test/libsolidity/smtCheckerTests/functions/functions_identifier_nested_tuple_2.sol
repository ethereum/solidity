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
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
