contract C {
	function g(uint i) public {
		require(address(this).balance == 100);
		this.h{value: i}();
		assert(address(this).balance == 100); // should hold
		assert(address(this).balance == 90); // should fail
	}

	function h() external payable {}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (162-197): CHC: Assertion violation happens here.
