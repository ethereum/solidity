pragma experimental SMTChecker;

contract C
{
	function f(bool x) public pure { require(x); for (;x;) {} }
}
// ----
// Warning: (98-99): Condition is always true.
