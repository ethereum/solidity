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
		// Disabled because of Spacer nondet
		/*
		assert(g(0, 0) == 1);
		assert(g(0, 1) == 0);
		assert(g(1, 0) == 1);
		assert(g(2, 3) == 8);
		assert(g(3, 10) == 59049);
		*/
	}
}
// ====
// SMTEngine: all
