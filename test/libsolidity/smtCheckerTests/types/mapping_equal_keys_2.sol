contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint y) public view {
		assert(x == y);
		assert(map[x] == map[y]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (86-100): CHC: Assertion violation happens here.
