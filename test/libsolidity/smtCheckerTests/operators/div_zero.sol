pragma experimental SMTChecker;

contract C {
	uint z = 0;
	uint x = 2 / z;
}
// ----
// Warning 4281: (69-74): CHC: Division by zero happens here.
