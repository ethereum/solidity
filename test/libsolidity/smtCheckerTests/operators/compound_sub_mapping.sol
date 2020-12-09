pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 100);
		map[p] = 200;
		map[p] -= map[p] - x;
		assert(map[p] >= 0);
		assert(map[p] < 90);
	}
}
// ----
// Warning 6328: (197-216): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 99\np = 0\n\n\nTransaction trace:\nconstructor()\nf(99, 0)
