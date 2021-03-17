pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		for(uint i = 0; i < 5; ++i) {
			x = x + 1;
		}
		// Disabled because of non-determinism in Spacer in Z3 4.8.9, check with next solver release.
		//assert(x > 0);
	}
}
// ----
