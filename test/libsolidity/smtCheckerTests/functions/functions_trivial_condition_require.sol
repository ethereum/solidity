contract C
{
	function f(bool x) public pure { x = true; require(x); }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (65-66): BMC: Condition is always true.
