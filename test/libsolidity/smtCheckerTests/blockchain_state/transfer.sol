contract C {
	function f(address payable a) public {
		require(address(this).balance > 1000);
		a.transfer(666);
		assert(address(this).balance > 100);
		// Fails.
		assert(address(this).balance > 500);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (166-201): CHC: Assertion violation happens here.
