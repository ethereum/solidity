contract C {
	uint[] a;
	function f() public {
		a.push();
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
