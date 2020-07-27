pragma experimental SMTChecker;

contract State {
	uint x;
	C c;
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
// ----
// Warning 6328: (299-313): Assertion violation happens here
