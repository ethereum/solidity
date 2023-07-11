contract C
{
	mapping (string => uint) map;
	function f(string memory s, uint x) public {
		map[s] = x;
		assert(x == map[s]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
