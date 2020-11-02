pragma experimental SMTChecker;

contract C {
	function test() public pure {
		uint x;
		if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7)) x = 1;
		if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7)) x = 2;
		assert(x == 0);
	}
}
// ----
// Warning 6838: (93-143): BMC: Condition is always false.
// Warning 6838: (158-208): BMC: Condition is always false.
