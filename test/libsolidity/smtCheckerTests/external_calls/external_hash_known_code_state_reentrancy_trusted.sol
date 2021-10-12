contract State {
	C c;
	constructor(C _c) {
		c = _c;
	}
	function f() public view returns (uint) {
		return c.g();
	}
}

contract C {
	address owner;
	uint y;
	State s;

	constructor() {
		owner = msg.sender;
		s = new State(this);
	}

	function f() public view {
		address prevOwner = owner;
		uint z = s.f();
		assert(z == y);
		assert(prevOwner == owner);
	}

	function g() public view returns (uint) {
		return y;
	}
}
// ====
// SMTContract: C
// SMTEngine: chc
// SMTExtCalls: trusted
// ----
// Warning 6328: (314-328): CHC: Assertion violation might happen here.
// Info 1180: Reentrancy property(ies) for :State:\n(<errorCode> = 0)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(z == y)\n<errorCode> = 2 -> Assertion failed at assert(prevOwner == owner)\n
