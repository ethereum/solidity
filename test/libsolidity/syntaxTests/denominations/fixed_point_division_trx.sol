contract C {
	uint constant a = 4 trx / 3 hours;
	ufixed constant b = ufixed(4 trx / 3 hours);
}
// ----
// TypeError: (32-47): Type rational_const 10000 / 27 is not implicitly convertible to expected type uint256. Try converting to type ufixed256x74 or use an explicit conversion.
