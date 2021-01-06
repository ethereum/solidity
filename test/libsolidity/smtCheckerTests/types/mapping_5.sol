pragma experimental SMTChecker;

contract C
{
	mapping (address => uint) map;
	function f(address a, uint x) public view {
		assert(x != map[a]);
	}
}
// ----
// Warning 6328: (125-144): CHC: Assertion violation happens here.\nCounterexample:\n\na = 38\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(38, 0)
