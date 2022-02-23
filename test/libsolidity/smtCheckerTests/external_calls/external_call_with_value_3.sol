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
// Warning 6328: (150-183): CHC: Assertion violation might happen here.
// Warning 6328: (202-236): CHC: Assertion violation happens here.\nCounterexample:\n\ni = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    i.f{value: 20}() -- untrusted external call
// Warning 4661: (150-183): BMC: Assertion violation happens here.
