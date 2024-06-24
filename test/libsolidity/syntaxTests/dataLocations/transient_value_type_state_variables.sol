contract D { }

contract C {
	address transient a;
	bool transient b;
	D transient d;
	uint transient x;
	bytes32 transient y;
}
// ----
// UnimplementedFeatureError 6715: (30-49): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (52-68): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (71-84): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (87-103): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (106-125): Transient storage is not yet implemented.
