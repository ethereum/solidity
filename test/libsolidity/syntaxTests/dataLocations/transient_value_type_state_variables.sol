contract D { }

contract C {
	address transient a;
	bool transient b;
	D transient d;
	uint transient x;
	bytes32 transient y;
}
// ----
// UnimplementedFeatureError 1834: Transient storage variables are not supported.
