contract C {
	mapping (uint => uint[]) map;
	function f() public view {
		assert(map[0].length == map[1].length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(true && (map[1].length <= 0))\n
