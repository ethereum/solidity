contract C {
	mapping (uint => uint[]) map;
	function f(uint x, uint y) public view {
		require(x == y);
		assert(map[x].length == map[y].length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
