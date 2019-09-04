pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 100);
		map[p] = 100;
		map[p] += map[p] + x;
		assert(map[p] < 300);
		assert(map[p] < 110);
	}
}
// ----
// Warning: (198-218): Assertion violation happens here
