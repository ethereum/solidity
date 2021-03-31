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
// Warning 6328: (206-220): CHC: Assertion violation happens here.\nCounterexample:\nlocked = false\ntarget = 0\n\nTransaction trace:\nC.constructor()\nState: locked = true\nC.call(0)\n    D(target).e() -- untrusted external call, synthesized as:\n        C.broken() -- reentrant call
