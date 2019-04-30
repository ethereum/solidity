pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 100);
		require(map[p] == 200);
		map[p] -= map[p] - x;
		assert(map[p] >= 0);
		assert(map[p] < 90);
	}
}
// ----
// Warning: (207-226): Assertion violation happens here
