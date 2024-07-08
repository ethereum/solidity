struct S {
	uint x;
	address a;
}

contract C {
	S transient s;
}
// ====
// EVMVersion: >=cancun
// ----
// UnimplementedFeatureError 1834: Transient data location is only supported for value types.
