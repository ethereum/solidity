contract A {
	function foo() internal view virtual returns (uint256) {}
}
contract B is A {
	function foo() internal pure override virtual returns (uint256) {}
}
// ----
