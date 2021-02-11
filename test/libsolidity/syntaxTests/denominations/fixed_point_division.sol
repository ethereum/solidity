contract C {
	uint constant a = 4 ether / 3 hours;
	ufixed constant b = ufixed(4 ether / 3 hours);
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
