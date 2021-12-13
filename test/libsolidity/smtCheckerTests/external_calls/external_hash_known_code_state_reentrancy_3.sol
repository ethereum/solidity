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
		// s.f() cannot call zz() because it is `view`
		// and zz modifies the state.
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
// Info 1180: Contract invariant(s) for :C:\n((insidef || (z <= 0)) && (y <= 0))\nReentrancy property(ies) for :C:\n((!insidef || !(<errorCode> >= 2)) && (insidef' || !insidef) && (!(y <= 0) || (y' <= 0)))\n((!insidef || !(<errorCode> >= 3)) && (insidef' || !insidef))\n<errorCode> = 0 -> no errors\n<errorCode> = 2 -> Assertion failed at assert(z == y)\n<errorCode> = 3 -> Assertion failed at assert(prevOwner == owner)\n
