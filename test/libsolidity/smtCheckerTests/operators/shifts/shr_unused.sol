pragma experimental SMTChecker;

contract C {
	function f() public pure {
		fixed x;
		assert(x >>> 6 == 0);
	}
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
