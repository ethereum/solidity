contract C {
	function g() public {
		require(address(this).balance == 100);
		this.h{value: 10}();
		assert(address(this).balance == 100); // should hold
		assert(address(this).balance == 90); // should fail
	}

	function h() external payable {}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (157-192): CHC: Assertion violation happens here.
