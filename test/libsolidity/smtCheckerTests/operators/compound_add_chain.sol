contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		uint c = 7;
		a += b += c;
		assert(b ==  10 && a == 11);
		a += (b += c);
		assert(b ==  17 && a == 28);
		a += a += a;
		assert(a == 112);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
