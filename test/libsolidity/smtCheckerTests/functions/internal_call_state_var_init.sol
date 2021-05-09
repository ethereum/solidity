contract c {
	bool b = (f() == 0) && (f() == 0);
	function f() internal returns (uint) {}
}
// ====
// SMTEngine: all
// ----
