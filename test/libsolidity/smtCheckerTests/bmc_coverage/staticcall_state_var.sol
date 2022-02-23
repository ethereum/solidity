contract C {
	uint x;
	function f(address _a) public {
		x = 2;
		_a.staticcall("");
		assert(x == 2); // should hold
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 9302: (66-83): Return value of low-level calls not used.
