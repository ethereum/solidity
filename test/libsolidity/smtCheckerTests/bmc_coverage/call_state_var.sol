contract C {
	uint x;
	function f(address _a) public {
		x = 2;
		_a.call("");
		assert(x == 2); // should fail
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 9302: (66-77): Return value of low-level calls not used.
// Warning 4661: (81-95): BMC: Assertion violation happens here.
