contract C {
	function g(address payable i) public {
		require(address(this).balance == 100);
		i.call{value: 10}("");
		assert(address(this).balance == 90); // should hold
		assert(address(this).balance == 100); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 9302: (96-117): Return value of low-level calls not used.
// Warning 6328: (121-156): CHC: Assertion violation might happen here.
// Warning 6328: (175-211): CHC: Assertion violation happens here.
// Warning 4661: (121-156): BMC: Assertion violation happens here.
