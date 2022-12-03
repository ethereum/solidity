contract C {
	function f() public view {
		abi.encode(this.f);
	}
}
// ====
// SMTEngine: all
// ----
