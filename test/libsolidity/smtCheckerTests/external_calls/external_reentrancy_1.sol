interface D { function e() external; }

contract C {
	bool locked = true;

	function call(address target) public {
		locked = false;
		D(target).e();
		locked = true;
	}

	function broken() public view {
		assert(locked);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (206-220): CHC: Error trying to invoke SMT solver.
// Warning 6328: (206-220): CHC: Assertion violation might happen here.
// Warning 4661: (206-220): BMC: Assertion violation happens here.
