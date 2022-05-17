contract C {
	event Nudge();
	event SomeArgs(uint, uint);
	event Caller(address, uint);
	function f() payable external {
		emit Nudge();
		emit SomeArgs(134, 567);
		emit Caller(msg.sender, msg.value);
	}
	function g_data() pure internal returns (uint) {
		assert(true);
	}
	function g() external {
		emit SomeArgs(g_data(), g_data());
	}
	bool x = true;
	function h_data() view internal returns (uint) {
		assert(x);
	}
	function h() external {
		x = false;
		emit SomeArgs(h_data(), h_data());
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (247-251): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (397-401): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (407-416): CHC: Assertion violation happens here.\nCounterexample:\nx = false\n\nTransaction trace:\nC.constructor()\nState: x = true\nC.h()\n    C.h_data() -- internal call
