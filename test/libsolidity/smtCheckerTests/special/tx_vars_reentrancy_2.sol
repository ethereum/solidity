interface I {
	function f() external payable;
}

contract C {
	function g(I _i) public payable {
		uint x = address(this).balance;
		_i.f{ value: 100 }();
		assert(x == address(this).balance); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 1218: (157-191): CHC: Error trying to invoke SMT solver.
// Warning 6328: (157-191): CHC: Assertion violation might happen here.
// Warning 4661: (157-191): BMC: Assertion violation happens here.
