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
// Warning: (139-142): Assertion checker does not yet support the type of this variable.
// Warning: (280-304): Assertion violation happens here
