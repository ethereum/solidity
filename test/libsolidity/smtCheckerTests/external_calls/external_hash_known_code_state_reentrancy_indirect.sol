contract Other {
	C c;
	function h() public {
		c.setOwner(address(0));
	}
}

contract State {
	uint x;
	Other o;
	C c;
	function f() public returns (uint) {
		o.h();
		return c.g();
	}
}

contract C {
	address owner;
	uint y;
	State s;

	constructor() {
		owner = msg.sender;
	}

	function setOwner(address _owner) public {
		owner = _owner;
	}

	function f() public {
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
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreOS: macos
// ----
// Warning 6328: (419-433): CHC: Assertion violation happens here.
// Warning 6328: (437-463): CHC: Assertion violation happens here.
