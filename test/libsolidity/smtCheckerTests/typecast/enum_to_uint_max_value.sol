contract C
{
	enum D { Left, Right }
	function f(D _a) public pure {
		uint x = uint(_a);
		assert(x < 10);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
