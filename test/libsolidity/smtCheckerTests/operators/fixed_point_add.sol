contract test {
	function f() internal pure {
		ufixed a = uint64(1) + ufixed(2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (48-56): Unused local variable.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
