contract Other {
	function h(C c) public {
		c.setOwner(address(0));
	}
}

contract State {
	uint x;
	Other o;
	constructor() {
		o = new Other();
	}
	function f(C c) public returns (uint) {
		o.h(c);
		return c.g();
	}
}

contract C {
	address owner;
	uint y;
	State s;

	constructor() {
		owner = msg.sender;
		s = new State();
	}

	function setOwner(address _owner) public {
		owner = _owner;
	}

	function f() public {
		address prevOwner = owner;
		uint z = s.f(this);
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
// Warning 6328: (509-535): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
