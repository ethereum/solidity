contract State {
	function f(uint _x) public pure returns (uint) {
		assert(_x < 100);
		return _x;
	}
}
contract C {
	State s;
	uint z = s.f(2);

	function f() public view {
		assert(z == 2); // should hold in trusted mode
	}
}
// ====
// SMTContract: C
// SMTEngine: all
// SMTExtCalls: trusted
// SMTIgnoreInv: yes
// ----
// Warning 6328: (69-85): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 100\n = 0\n\nTransaction trace:\nState.constructor()\nState.f(100)
