interface I {
	function f() external payable;
}

contract C {
	function g(I i) public {
		require(address(this).balance > 100);
		i.f{value: 20}();
		assert(address(this).balance > 0); // should hold
		assert(address(this).balance == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 1218: (202-236): CHC: Error trying to invoke SMT solver.
// Warning 6328: (150-183): CHC: Assertion violation might happen here.
// Warning 6328: (202-236): CHC: Assertion violation might happen here.
// Warning 4661: (150-183): BMC: Assertion violation happens here.
// Warning 4661: (202-236): BMC: Assertion violation happens here.
