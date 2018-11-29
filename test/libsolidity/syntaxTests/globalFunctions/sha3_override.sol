contract C
{
	function sha3() public pure returns (bool) {
		return true;
	}
	function f() public pure returns (bool) {
		return sha3();
	}
}
// ----
// Warning: (14-76): This declaration shadows a builtin symbol.
