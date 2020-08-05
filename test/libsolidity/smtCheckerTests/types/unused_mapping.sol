pragma experimental SMTChecker;

contract C {
	uint x;
	uint y;
	mapping (address => bool) public never_used;

	function inc() public {
		require(x < 10);
		require(y < 10);

		if(x == 0) x = 0; // noop state var read
		x++;
		y++;
		assert(y == x);
	}
}
