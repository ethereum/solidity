contract C {
	mapping (uint => uint[]) map;
	function f() public view {
		assert(map[0].length == map[1].length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
