pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 10);
		map[p] = 10;
		map[p] *= map[p] + x;
		assert(map[p] <= 190);
		assert(map[p] < 50);
	}
}
// ----
// Warning 6328: (197-216): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\np = 0\n\n\nTransaction trace:\nconstructor()\nf(0, 0)
