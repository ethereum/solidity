contract C {
	bool once;
	function f() public payable {
		require(!once);
		once = true;
		require(msg.value == 10);
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
		g();
	}
	function g() internal view {
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
		h();
	}
	function h() internal view {
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (173-208): CHC: Assertion violation happens here.
// Warning 6328: (321-356): CHC: Assertion violation happens here.
// Warning 6328: (469-504): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
