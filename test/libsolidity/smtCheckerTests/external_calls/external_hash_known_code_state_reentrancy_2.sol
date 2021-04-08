contract State {
	uint x;
	C c;
	function f() public returns (uint) {
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
// SMTEngine: all
// ----
// Warning 2018: (33-88): Function state mutability can be restricted to view
// Warning 6328: (367-381): CHC: Assertion violation happens here.\nCounterexample:\nowner = 0, y = 0, z = 3, s = 0, insidef = true\nprevOwner = 0\n\nTransaction trace:\nC.constructor()\nState: owner = 0, y = 0, z = 0, s = 0, insidef = false\nC.f()\n    s.f() -- untrusted external call, synthesized as:\n        C.zz() -- reentrant call
