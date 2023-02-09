contract C {
	function f(address _a) public view {
		uint b1 = address(this).balance;
		_a.staticcall("");
		uint b2 = address(this).balance;
		assert(b1 == b2); // should hold
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 9302: (88-105): Return value of low-level calls not used.
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
