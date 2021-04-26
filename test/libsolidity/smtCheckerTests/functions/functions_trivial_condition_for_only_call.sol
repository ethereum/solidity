contract C
{
	function f(bool x) public pure { for (;x;) {} }
	function g() public pure { f(true); }
}
// ====
// SMTEngine: all
// ----
