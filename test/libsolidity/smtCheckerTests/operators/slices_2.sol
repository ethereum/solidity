contract e {
	function f(uint[] calldata) internal {}
	function h(uint[] calldata c) external { f(c[:]); }
}
// ====
// SMTEngine: all
// ----
