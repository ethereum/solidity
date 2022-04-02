contract C
{
	function f(bool x) public pure { require(x); while (x) {} }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (66-67='x'): BMC: Condition is always true.
