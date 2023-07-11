contract C
{
	mapping (uint => uint) map;
	function f(uint x) public {
		map[2] = x;
		assert(x == map[2]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
