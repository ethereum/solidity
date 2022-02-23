contract A {
	function foo() internal view virtual returns (uint256) {}
}
contract B is A {
	function foo() internal pure override virtual returns (uint256) {}
}
contract C is A {
	function foo() internal view override virtual returns (uint256) {}
}
contract D is B, C {
	function foo() internal pure override(B, C) virtual returns (uint256) {}
}
contract E is C, B {
	function foo() internal pure override(B, C) virtual returns (uint256) {}
}
// ----
