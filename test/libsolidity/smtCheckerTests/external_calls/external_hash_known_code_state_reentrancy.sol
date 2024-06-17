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
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (266-280): CHC: Assertion violation happens here.\nCounterexample:\nowner = 0x0, y = 0, s = 0\nprevOwner = 0x0\nz = 1\n\nTransaction trace:\nC.constructor(){ msg.sender: 0x0 }\nState: owner = 0x0, y = 0, s = 0\nC.f()\n    s.f() -- untrusted external call
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
