contract Simple {
	function f() public pure {
		uint x = 10;
		uint y;
		while (y < x)
		{
			++y;
			x = 0;
			while (x < 10)
				++x;
			assert(x == 10);
		}
		// Removed because of Spacer nondeterminism.
		//assert(y == x);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
