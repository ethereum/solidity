contract C {
	function f() public pure {
		fixed x;
		assert(x >>> 6 == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// UnimplementedFeatureError 1834: Not yet implemented - FixedPointType.
