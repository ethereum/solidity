struct S {
	uint x;
	address a;
}

contract C {
	S transient s;
}
// ====
// stopAfter: analysis
// ----
// UnimplementedFeatureError 1834: Transient data location is only supported for value types.
