interface I {
	function f() external;
}

contract C {
	function g(I _i) public payable {
		uint x = address(this).balance;
		_i.f();
		assert(x == address(this).balance); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 1218: (135-169): CHC: Error trying to invoke SMT solver.
// Warning 6328: (135-169): CHC: Assertion violation might happen here.
// Warning 4661: (135-169): BMC: Assertion violation happens here.
