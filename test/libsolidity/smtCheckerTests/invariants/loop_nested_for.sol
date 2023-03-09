contract Simple {
	function f() public pure {
		uint x;
		uint y;
		for (x = 10; y < x; ++y)
		{
			for (x = 0; x < 10; ++x) {}
			assert(x == 10);
		}
		// Disabled because of Spacer nondeterminism.
		//assert(y == x);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
