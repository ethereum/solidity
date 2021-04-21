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
