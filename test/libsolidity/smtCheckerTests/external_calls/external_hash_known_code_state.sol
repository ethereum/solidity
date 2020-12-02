pragma experimental SMTChecker;

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
// ----
// Warning 6328: (528-565): CHC: Assertion violation happens here.\nCounterexample:\nowner = 1, y = 0, z = 0, s = 0\n\n\n\nTransaction trace:\nconstructor()\nState: owner = 1, y = 0, z = 0, s = 0\ninv()
