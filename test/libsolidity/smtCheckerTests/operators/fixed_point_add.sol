pragma experimental SMTChecker;
contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ----
// Warning 2072: (80-88): Unused local variable.
// Warning 4984: (91-112): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 5084: (103-112): Type conversion is not yet fully supported and might yield false positives.
