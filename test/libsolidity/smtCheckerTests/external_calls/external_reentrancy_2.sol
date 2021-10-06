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
// Warning 6328: (117-131): CHC: Assertion violation happens here.\nCounterexample:\nlocked = false\ntarget = 0x0\n\nTransaction trace:\nC.constructor()\nState: locked = true\nC.call(0x0)\n    D(target).e() -- untrusted external call, synthesized as:\n        C.call(0x0) -- reentrant call
