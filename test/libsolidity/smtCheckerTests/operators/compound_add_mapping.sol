pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 100);
		require(map[p] == 100);
		map[p] += map[p] + x;
		assert(map[p] < 300);
		assert(map[p] < 110);
	}
}
// ----
// Warning: (208-228): Assertion violation happens here
