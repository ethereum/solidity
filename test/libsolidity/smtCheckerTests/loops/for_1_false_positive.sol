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
// ====
// SMTEngine: all
// ----
// Warning 4984: (106-111): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (106-111): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
