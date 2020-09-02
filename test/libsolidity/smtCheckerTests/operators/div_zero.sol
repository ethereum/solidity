pragma experimental SMTChecker;

contract C {
	uint z = 0;
	uint x = 2 / z;
}
// ----
// Warning 6084: (69-74): Division by zero happens here.
