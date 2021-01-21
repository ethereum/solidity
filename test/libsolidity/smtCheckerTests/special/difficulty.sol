pragma experimental SMTChecker;

contract C
{
	function f(uint difficulty) public view {
		assert(block.difficulty == difficulty);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (91-129): CHC: Assertion violation happens here.
