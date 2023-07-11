contract C
{
	enum D { Left, Right }
	function f(D a) public pure {
		assert(a == D.Left || a == D.Right);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
