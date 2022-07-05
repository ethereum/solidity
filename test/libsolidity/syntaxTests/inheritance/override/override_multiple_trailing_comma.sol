abstract contract A {
	function foo() internal virtual returns (uint256);
}
abstract contract X is A {
	function foo() internal override(A,) virtual returns (uint256);
}
// ----
