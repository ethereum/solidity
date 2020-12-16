pragma experimental SMTChecker;
contract C {
	uint z = 1;
	uint w = z - 3;
}
// ====
// SMTEngine: bmc
// ----
// Warning 4144: (68-73): BMC: Underflow (resulting value less than 0) happens here.
