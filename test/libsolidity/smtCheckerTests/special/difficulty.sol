pragma experimental SMTChecker;

contract C
{
	function f(uint difficulty) public view {
		assert(block.difficulty == difficulty);
	}
}
// ----
// Warning 4661: (91-129): Assertion violation happens here
