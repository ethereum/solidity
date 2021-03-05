pragma experimental SMTChecker;

contract C
{
	function f(uint difficulty) public view {
		assert(block.difficulty == difficulty);
	}
}
// ----
// Warning 6328: (91-129): CHC: Assertion violation happens here.\nCounterexample:\n\ndifficulty = 38\n\nTransaction trace:\nC.constructor()\nC.f(38)
