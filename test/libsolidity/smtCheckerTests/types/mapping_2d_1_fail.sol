pragma experimental SMTChecker;

contract C
{
	mapping (uint => mapping (uint => uint)) map;
	function f(uint x) public {
		x = 41;
		map[13][14] = 42;
		assert(x == map[13][14]);
	}
}
// ----
// Warning 6328: (154-178): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 41\n\n\nTransaction trace:\nconstructor()\nf(0)
