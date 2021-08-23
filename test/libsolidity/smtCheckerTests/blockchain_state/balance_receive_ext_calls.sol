interface I {
	function ext() external;
}

contract C {
	function f(I _i) public {
		uint x = address(this).balance;
		_i.ext();
		assert(address(this).balance == x); // should fail
		assert(address(this).balance >= x); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (131-165): CHC: Error trying to invoke SMT solver.
// Warning 6328: (131-165): CHC: Assertion violation might happen here.
// Warning 4661: (131-165): BMC: Assertion violation happens here.
