pragma experimental SMTChecker;

contract C
{
	function f(bool x) public pure { x = true; require(x); }
}
// ----
// Warning: (98-99): Condition is always true.
