contract D { }

contract C {
	int transient x = -99;
	address transient a = address(0xABC);
	bool transient b = x > 0 ? false : true;
}
// ----
// UnimplementedFeatureError 6715: (30-51): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (54-90): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (93-132): Transient storage is not yet implemented.
