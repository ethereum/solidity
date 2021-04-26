contract C
{
	function f(bool x) public pure { require(x); }
	function g() public pure { f(true); }
}
// ====
// SMTEngine: all
// ----
