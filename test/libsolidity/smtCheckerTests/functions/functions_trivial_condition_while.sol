pragma experimental SMTChecker;

contract C
{
	function f(bool x) public pure { require(x); while (x) {} }
}
// ----
// Warning: (99-100): Condition is always true.
