pragma experimental SMTChecker;

contract C {
	uint z = 0;
	uint x = 2 / z;
}
// ----
// Warning 6084: (69-74): BMC: Division by zero happens here.
