contract C
{
	function f(bool x) public pure { require(x); if (x) {} }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (63-64='x'): BMC: Condition is always true.
