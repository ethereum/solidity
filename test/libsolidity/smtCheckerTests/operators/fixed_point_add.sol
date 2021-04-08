contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (48-56): Unused local variable.
