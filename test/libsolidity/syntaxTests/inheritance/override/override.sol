abstract contract A {
	function test() internal virtual returns (uint256);
	function test2() internal virtual returns (uint256);
}
contract X is A {
	function test() internal override returns (uint256) {}
	function test2() internal override(A) returns (uint256) {}
}
// ----
