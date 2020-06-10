pragma experimental SMTChecker;
contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ----
// Warning: (80-88): Unused local variable.
// Warning: (91-100): Type conversion is not yet fully supported and might yield false positives.
// Warning: (103-112): Type conversion is not yet fully supported and might yield false positives.
// Warning: (91-112): Underflow (resulting value less than 0) happens here
// Warning: (91-112): Overflow (resulting value larger than 2**256 - 1) happens here
