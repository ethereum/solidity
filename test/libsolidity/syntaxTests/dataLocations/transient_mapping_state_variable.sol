contract C {
	mapping(uint => uint) transient y;
}
// ====
// stopAfter: analysis
// ----
// UnimplementedFeatureError 1834: Transient data location is only supported for value types.
