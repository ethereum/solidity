contract C {
	function f() public pure {
		fixed x;
		assert(x >>> 6 == 0);
	}
}
// ====
// SMTEngine: all
// ----
// TypeError 2271: (61-68): Operator >>> not compatible with types fixed128x18 and int_const 6. Arithmetic operators on fixed point types are not yet supported.
