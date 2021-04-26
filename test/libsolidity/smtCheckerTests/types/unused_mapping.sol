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
		// Commented out because of nondeterminism in Spacer in Z3 4.8.9
		//assert(y == x);
	}
}
// ====
// SMTEngine: all
// ----
