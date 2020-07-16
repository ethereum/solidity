contract A {
	function foo() external pure virtual returns (uint256) {}
}
contract B is A {
	function foo() external pure override virtual returns (uint256) {}
}
contract C is A {
	function foo() external view override virtual returns (uint256) {}
}
contract D is B, C {
	function foo() external override(B, C) virtual returns (uint256) {}
}
contract E is C, B {
	function foo() external pure override(B, C) virtual returns (uint256) {}
}
contract F is C, B {
	function foo() external payable override(B, C) virtual returns (uint256) {}
}
// ----
// TypeError 6959: (181-247): Overriding function changes state mutability from "pure" to "view".
// TypeError 6959: (272-339): Overriding function changes state mutability from "pure" to "nonpayable".
// TypeError 6959: (272-339): Overriding function changes state mutability from "view" to "nonpayable".
// TypeError 6959: (461-536): Overriding function changes state mutability from "view" to "payable".
// TypeError 6959: (461-536): Overriding function changes state mutability from "pure" to "payable".
