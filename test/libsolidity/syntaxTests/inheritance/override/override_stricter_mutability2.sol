contract A {
	function foo() internal virtual returns (uint256) {}
}
contract B is A {
	function foo() internal view override virtual returns (uint256) {}
}
// ----
