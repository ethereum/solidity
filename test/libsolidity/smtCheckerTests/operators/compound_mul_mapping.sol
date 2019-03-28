pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 10);
		require(map[p] == 10);
		map[p] *= map[p] + x;
		assert(map[p] <= 190);
		assert(map[p] < 50);
	}
}
// ----
// Warning: (207-226): Assertion violation happens here
