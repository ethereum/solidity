contract State {
	C c;
	constructor(C _c) {
		c = _c;
	}
	function f() public returns (uint) {
		c.setOwner(address(0));
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

	function setOwner(address _owner) public {
		owner = _owner;
	}

	function f() public {
		address prevOwner = owner;
		uint z = s.f();
		assert(z == y); // should hold
		assert(prevOwner == owner); // should not hold because of reentrancy
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
// Warning 6328: (396-410): CHC: Assertion violation might happen here.
// Warning 6328: (429-455): CHC: Assertion violation happens here.
