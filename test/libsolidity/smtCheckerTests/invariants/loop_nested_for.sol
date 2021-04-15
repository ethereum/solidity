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
