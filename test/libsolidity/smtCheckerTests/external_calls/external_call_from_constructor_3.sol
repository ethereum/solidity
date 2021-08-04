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
		assert(z == 2); // should fail since we don't trust s.f's code
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (69-85): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 100\n = 0\n\nTransaction trace:\nState.constructor()\nState.f(100)
// Warning 6328: (203-217): CHC: Assertion violation happens here.\nCounterexample:\ns = 0, z = 0\n\nTransaction trace:\nC.constructor()\nState: s = 0, z = 0\nC.f()
