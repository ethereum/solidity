contract C {
	function g() external {
		f();
	}
}
function f() {}
// ====
// SMTEngine: all
// ----
