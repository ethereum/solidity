pragma experimental SMTChecker;

contract D
{
	function g(uint x) public;
}

contract C
{
	mapping (uint => uint) map;
	function f(uint y, D d) public {
		require(map[0] == map[1]);
		assert(map[0] == map[1]);
		d.g(y);
		// Storage knowledge is cleared after an external call.
		assert(map[0] == map[1]);
	}
}
// ----
// TypeError: (33-75): Contract "D" should be marked as abstract.
