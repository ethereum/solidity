interface I {
	function f() external payable;
}

contract C {
	function g(I i) public {
		require(address(this).balance == 100);
		i.f{value: 10}();
		assert(address(this).balance == 90); // should hold
		// Disabled due to Spacer nondeterminism
		//assert(address(this).balance == 100); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (151-186): CHC: Assertion violation might happen here.
// Warning 4661: (151-186): BMC: Assertion violation happens here.
