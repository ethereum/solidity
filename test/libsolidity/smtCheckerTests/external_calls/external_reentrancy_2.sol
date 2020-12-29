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
// Warning 6328: (150-164): CHC: Assertion violation happens here.\nCounterexample:\nlocked = false\ntarget = 0\n\n\nTransaction trace:\nC.constructor()\nState: locked = true\nC.call(0)
