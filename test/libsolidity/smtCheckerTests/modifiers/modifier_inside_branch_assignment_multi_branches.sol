pragma experimental SMTChecker;

contract C {
	uint x;
	address owner;

	modifier onlyOwner {
		if (msg.sender == owner) {
			require(x > 0);
			_;
		}
	}

	function f() public onlyOwner {
		x -= 1;
		h();
	}
	function h() public onlyOwner {
		require(x < 10000);
		x += 2;
	}
	function g(uint y) public {
		require(y > 0 && y < 10000);
		require(msg.sender == owner);
		x = y;
		if (y > 1) {
			f();
			assert(x == y + 1);
		}
		// Fails for {y = 0, x = 0}.
		assert(x == 0);
	}
}
// ----
// Warning: (461-475): Assertion violation happens here
