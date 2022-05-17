contract C {
	function g(address payable i) public {
		require(address(this).balance == 100);
		i.call{value: 0}("");
		assert(address(this).balance == 100); // should hold
		assert(address(this).balance == 20); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (96-116): Return value of low-level calls not used.
// Warning 6328: (120-156): CHC: Assertion violation might happen here.
// Warning 6328: (175-210): CHC: Assertion violation happens here.
// Warning 4661: (120-156): BMC: Assertion violation happens here.
