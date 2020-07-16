contract A {
	function foo() public view virtual returns (uint256) {}
}
contract B is A {
	function foo() public payable override virtual returns (uint256) {}
}
// ----
// TypeError 6959: (91-158): Overriding function changes state mutability from "view" to "payable".
