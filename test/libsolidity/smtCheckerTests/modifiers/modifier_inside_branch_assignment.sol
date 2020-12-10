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
// Warning 6328: (287-300): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, owner = 0\ny = 1\n\n\nTransaction trace:\nconstructor()\nState: x = 0, owner = 0\ng(1)
