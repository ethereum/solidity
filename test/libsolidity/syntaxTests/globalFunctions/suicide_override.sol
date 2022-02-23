contract C
{
	function suicide() public pure returns (bool) {
		return true;
	}
	function f() public pure returns (bool) {
		return suicide();
	}
}
// ----
// Warning 2319: (14-79): This declaration shadows a builtin symbol.
