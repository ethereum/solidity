pragma experimental SMTChecker;

contract C {
	uint x;
	address owner;

	modifier onlyOwner {
		if (msg.sender == owner) _;
	}

	function f() public onlyOwner {
		x = 0;
	}
	function g(uint y) public {
		x = 1;
		if (y > 0)
			f();
		// Fails for {y = >0, msg.sender == owner, x = 0}.
		assert(x > 0);
	}
}
// ----
// Warning: (287-300): Assertion violation happens here
