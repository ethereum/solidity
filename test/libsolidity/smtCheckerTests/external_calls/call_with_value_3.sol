contract C {
	function g(address payable i) public {
		require(address(this).balance > 100);
		i.call{value: 20}("");
		assert(address(this).balance > 0); // should hold
		// Disabled due to Spacer nondeterminism
		//assert(address(this).balance == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (95-116): Return value of low-level calls not used.
// Warning 6328: (120-153): CHC: Assertion violation might happen here.
// Warning 4661: (120-153): BMC: Assertion violation happens here.
