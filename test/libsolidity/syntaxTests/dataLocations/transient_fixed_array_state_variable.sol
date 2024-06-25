contract C {
	uint[3] transient x;
}
// ----
// UnimplementedFeatureError 1834: Transient data location is only supported for value types.
