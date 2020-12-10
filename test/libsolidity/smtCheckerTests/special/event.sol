pragma experimental SMTChecker;

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
// ----
// Warning 6321: (280-284): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (430-434): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (440-449): CHC: Assertion violation happens here.\nCounterexample:\nx = false\n\n\n\nTransaction trace:\nconstructor()\nState: x = true\nh()
