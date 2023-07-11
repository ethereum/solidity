contract C {
	mapping (uint => uint[][]) map;
	function f(uint x, uint y) public {
		require(x == y);
		map[x].push();
		assert(map[x][0].length == map[y][0].length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
