contract C {
	uint x;
	address owner;

	modifier onlyOwner {
		if (msg.sender == owner) _;
	}

	function f() public onlyOwner {
		x = 0;
	}
	function g(uint y) public {
		x = 1;
		if (y > 0)
			f();
		// Fails for {y = >0, msg.sender == owner, x = 0}.
		assert(x > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (254-267): CHC: Error trying to invoke SMT solver.
// Warning 6328: (254-267): CHC: Assertion violation might happen here.
// Warning 4661: (254-267): BMC: Assertion violation happens here.
