contract C {
	function f() public {}
	function g() public {}

	function test() public {
		true ? f() : g();
	}
}
// ====
// SMTEngine: all
// ----
