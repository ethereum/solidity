contract State {
	function f(uint _x) public pure returns (uint) {
		assert(_x < 100);
		return _x;
	}
}
contract C {
	State s;
	uint z;

	constructor() {
		z = s.f(2);
	}

	function f() public view {
		assert(z == 2); // should hold in trusted mode
	}
}
// ====
// SMTContract: C
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Warning 6328: (69-85): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 100\n = 0\n\nTransaction trace:\nState.constructor()\nState.f(100)
// Info 1180: Contract invariant(s) for :C:\n(!(z >= 3) && !(z <= 1))\n
