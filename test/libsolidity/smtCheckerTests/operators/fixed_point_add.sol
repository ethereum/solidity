pragma experimental SMTChecker;
contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ----
// Warning 2072: (80-88): Unused local variable.
// Warning 5084: (91-100): Type conversion is not yet fully supported and might yield false positives.
// Warning 5084: (103-112): Type conversion is not yet fully supported and might yield false positives.
// Warning 4144: (91-112): Underflow (resulting value less than 0) happens here
// Warning 2661: (91-112): Overflow (resulting value larger than 2**256 - 1) happens here
