contract C {
	function f(address _a) public {
		uint b1 = address(this).balance;
		_a.call("");
		uint b2 = address(this).balance;
		assert(b1 == b2); // should fail
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 9302: (83-94): Return value of low-level calls not used.
// Warning 4661: (133-149): BMC: Assertion violation happens here.
