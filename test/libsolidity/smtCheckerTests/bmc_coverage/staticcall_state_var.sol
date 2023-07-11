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
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
