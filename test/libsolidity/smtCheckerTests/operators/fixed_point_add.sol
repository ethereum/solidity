pragma experimental SMTChecker;
contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ----
// Warning 2072: (80-88): Unused local variable.
