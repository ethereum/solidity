contract C
{
	function f() public pure returns (bool) {
		bool sha3 = true;
		return sha3;
	}
}
// ----
// Warning 2319: (58-67): This declaration shadows a builtin symbol.
