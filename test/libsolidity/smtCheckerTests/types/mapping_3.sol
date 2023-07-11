contract C
{
	mapping (uint => uint) map;
	function f() public {
		map[1] = 111;
		uint x = map[2];
		map[1] = 112;
		assert(map[2] == x);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
