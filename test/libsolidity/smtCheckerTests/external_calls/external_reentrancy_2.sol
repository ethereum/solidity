interface D { function e() external; }

contract C {
	bool locked = true;

	function call(address target) public {
		assert(locked);
		locked = false;
		D(target).e();
		locked = true;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (117-131): CHC: Assertion violation happens here.
