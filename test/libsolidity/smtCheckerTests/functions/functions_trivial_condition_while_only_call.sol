contract C
{
	function f(bool x) public pure { while (x) {} }
	function g() public pure { f(true); }
}
// ====
// SMTEngine: all
// ----
