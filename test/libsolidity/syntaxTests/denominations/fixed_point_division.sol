contract C {
	uint constant a = 4 ether / 3 hours;
	ufixed constant b = ufixed(4 ether / 3 hours);
}
// ----
// TypeError 7407: (32-49): Type rational_const 10000000000000000 / 27 is not implicitly convertible to expected type uint256. Rational number is fractional, use an explicit conversion instead.
