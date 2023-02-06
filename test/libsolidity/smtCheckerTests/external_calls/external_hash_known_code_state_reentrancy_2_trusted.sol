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
	uint z;
	State s;
	bool insidef;

	constructor() {
		owner = msg.sender;
		s = new State(this);
	}

	function zz() public {
		require(insidef);
		z = 3;
	}

	function f() public {
		require(!insidef);
		address prevOwner = owner;
		insidef = true;
		s.f();
		assert(z == y);
		assert(prevOwner == owner);
		insidef = false;
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
// Info 1180: Contract invariant(s) for :C:\n((y <= 0) && (insidef || (z <= 0)))\nReentrancy property(ies) for :State:\n(<errorCode> = 0)\n<errorCode> = 0 -> no errors\n<errorCode> = 2 -> Assertion failed at assert(z == y)\n<errorCode> = 3 -> Assertion failed at assert(prevOwner == owner)\n
