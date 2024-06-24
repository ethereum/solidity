contract C {
	uint[] transient x;
}
// ----
// UnimplementedFeatureError 1834: Transient data location is only supported for value types.
