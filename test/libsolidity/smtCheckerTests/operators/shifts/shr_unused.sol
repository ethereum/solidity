contract C {
	function f() public pure {
		fixed x;
		assert(x >>> 6 == 0);
	}
}
// ====
// SMTEngine: all
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
