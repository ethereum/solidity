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

	function setOwner(address _owner) public {
		owner = _owner;
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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (435-461): CHC: Assertion violation happens here.
// Warning 6328: (594-631): CHC: Assertion violation happens here.
