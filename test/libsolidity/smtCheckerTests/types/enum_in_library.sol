library L
{
	enum D { Left, Right }
}

contract C
{
	enum E { Left, Right }
	function f(E _d) public pure {
		_d = E.Left;
		assert(_d == E.Left);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
