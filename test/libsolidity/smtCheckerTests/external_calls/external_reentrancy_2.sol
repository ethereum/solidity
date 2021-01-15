pragma experimental SMTChecker;

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
// ----
// Warning 6328: (150-164): CHC: Assertion violation happens here.\nCounterexample:\nlocked = false\ntarget = 0\n\nTransaction trace:\nC.constructor()\nState: locked = true\nC.call(0)\n    D(target).e() -- untrusted external call, synthesized as:\n        C.call(0) -- reentrant call
