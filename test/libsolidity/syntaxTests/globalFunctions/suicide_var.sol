contract C
{
	function f() public pure returns (bool) {
		bool suicide = true;
		return suicide;
	}
}
// ----
// Warning: (58-70): This declaration shadows a builtin symbol.
