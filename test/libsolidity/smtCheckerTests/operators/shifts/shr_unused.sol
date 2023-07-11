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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
