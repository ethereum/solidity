pragma experimental SMTChecker;

contract State {
	uint x;
	C c;
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
// ----
// Warning 6328: (381-395): Assertion violation happens here
// Warning 6328: (399-425): Assertion violation happens here
// Warning 5084: (116-126): Type conversion is not yet fully supported and might yield false positives.
