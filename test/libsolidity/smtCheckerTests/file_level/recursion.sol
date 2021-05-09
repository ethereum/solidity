function exp(uint base, uint exponent) pure returns (uint power) {
	if (exponent == 0)
		return 1;
	power = exp(base, exponent / 2);
	power *= power;
	if (exponent & 1 == 1)
		power *= base;
}

contract C {
	function g(uint base, uint exponent) internal pure returns (uint) {
		return exp(base, exponent);
	}
	function f() public pure {
		// All of these should hold but the SMTChecker can't prove them.
		assert(g(0, 0) == 1);
		assert(g(0, 1) == 0);
		assert(g(1, 0) == 1);
		assert(g(2, 3) == 8);
		assert(g(3, 10) == 59049);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (118-130): CHC: Division by zero might happen here.
// Warning 4984: (134-148): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (176-189): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (430-450): CHC: Assertion violation happens here.
// Warning 6328: (478-498): CHC: Assertion violation might happen here.
// Warning 6328: (502-527): CHC: Assertion violation might happen here.
// Warning 2661: (134-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (176-189): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (134-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (134-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 8065: (176-189): BMC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4661: (478-498): BMC: Assertion violation happens here.
// Warning 2661: (134-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4661: (502-527): BMC: Assertion violation happens here.
