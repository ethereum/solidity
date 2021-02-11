pragma experimental SMTChecker;
function f()pure {
	ufixed a = uint64(1) + ufixed(2);
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
