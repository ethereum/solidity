contract C
{
	function f(uint x, bool b) public pure {
		require(x < 100);
		while (x < 10) {
			if (b)
				x = x + 1;
			else
				x = 0;
		}
		// CHC proves it safe because
		// 1- if it doesn't go in the loop in the first place, x >= 10
		// 2- if it goes in the loop and b == true, x increases until >= 10
		// 3- if it goes in the loop and b == false, it's an infinite loop, therefore
		//    the assertion and the error are unreachable.
		assert(x > 0);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
