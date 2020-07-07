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

	constructor() public {
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
// ----
// Warning 4661: (442-468): Assertion violation happens here
// Warning 5084: (617-627): Type conversion is not yet fully supported and might yield false positives.
// Warning 4661: (601-638): Assertion violation happens here
