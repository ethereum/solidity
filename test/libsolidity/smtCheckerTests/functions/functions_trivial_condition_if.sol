pragma experimental SMTChecker;
contract C
{
	function f(bool x) public pure { require(x); if (x) {} }
}
// ----
// Warning: (95-96): Condition is always true.
