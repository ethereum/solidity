pragma experimental SMTChecker;
contract C
{
	function f(bool x) public pure { require(x); if (x) {} }
}
// ----
// Warning 6838: (95-96): BMC: Condition is always true.
