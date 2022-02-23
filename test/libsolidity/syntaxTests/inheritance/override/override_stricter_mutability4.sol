contract A {
	function foo() public payable virtual returns (uint256) {}
}
contract B is A {
	function foo() public view override virtual returns (uint256) {}
}
// ----
// TypeError 6959: (94-158): Overriding function changes state mutability from "payable" to "view".
