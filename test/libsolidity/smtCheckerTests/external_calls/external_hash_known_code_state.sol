contract State {
	uint x;
	function f() public returns (uint) {
		if (x == 0) x = 1;
		else if (x == 1) x = 2;
		else if (x == 2) x = 0;
		return x;
	}
}

contract C {
	address owner;
	uint y;
	uint z;
	State s;

	constructor() {
		owner = msg.sender;
	}

	function f() public {
		address prevOwner = owner;
		y = s.f();
		z = s.f();
		assert(prevOwner == owner);
	}

	function inv() public view {
		// This is safe but external calls do not yet support the state
		// of the called contract.
		assert(owner == address(0) || y != z);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (495-532): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n(((owner + ((- 1) * owner')) <= 0) && !(<errorCode> = 1) && ((owner + ((- 1) * owner')) >= 0))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(prevOwner == owner)\n<errorCode> = 3 -> Assertion failed at assert(owner == address(0) || y != z)\n
